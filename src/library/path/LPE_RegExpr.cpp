#include "LPE_RegExpr.h"

#include "Logger.h"

#include <fstream>

namespace PR {
/* Primarily based on 
	https://www.codeguru.com/cpp/cpp/cpp_mfc/parsing/article.php/c4093/Write-Your-Own-Regular-Expression-Parser.htm
* The initial implementation is changed to allow better dfa optimization and complements
*/
LPE_RegExpr::LPE_RegExpr()
{
	/*static const char* TYPES  = ".ERTBL";
	static const char* EVENTS = ".DS";

	for (uint32 i = 0; TYPES[i]; ++i) {
		for (uint32 j = 0; EVENTS[j]; ++j) {
			mInputSet.insert(LPE_Token(TYPES[i], EVENTS[j]));
		}
	}*/
}

void LPE_RegExpr::push(const LPE_Token& token)
{
	auto s1 = std::make_shared<LPE_RegState>();
	auto s2 = std::make_shared<LPE_RegState>();

	s1->addTransition(token, s2);

	FSATable table;
	table.push_back(s1);
	table.push_back(s2);

	s1->makeInitial(true);
	s2->makeFinal(true);

	mOperandStack.push(table);
	mInputSet.insert(token);
}

bool LPE_RegExpr::pop(FSATable& table)
{
	if (mOperandStack.empty())
		return false;

	table = mOperandStack.top();
	mOperandStack.pop();
	return true;
}

bool LPE_RegExpr::doConcat()
{
	FSATable a, b;
	if (!pop(b) || !pop(a))
		return false;

	for (auto& s : a) {
		if (s->isFinal())
			connect_to_initial(s, LPE_Token(), b);
	}

	// Remove final flag from a and initial flag from b
	remove_flags(a, false, true);
	remove_flags(b, true, false);

	a.insert(a.end(), b.begin(), b.end());

	mOperandStack.push(a);
	return true;
}

bool LPE_RegExpr::doUnion()
{
	FSATable a, b;
	if (!pop(b) || !pop(a))
		return false;

	auto start = std::make_shared<LPE_RegState>();
	auto end   = std::make_shared<LPE_RegState>();

	connect_to_initial(start, LPE_Token(), a);
	connect_to_initial(start, LPE_Token(), b);

	connect_from_final(end, LPE_Token(), a);
	connect_from_final(end, LPE_Token(), b);

	remove_flags(a);
	remove_flags(b);

	b.push_back(end);
	a.push_front(start);
	a.insert(a.end(), b.begin(), b.end());

	start->makeInitial(true);
	end->makeFinal(true);

	mOperandStack.push(a);
	return true;
}

bool LPE_RegExpr::repeatLast(uint32 min, uint32 max)
{
	FSATable a;
	if (!pop(a))
		return false;

	FSATable newFull;
	auto start = std::make_shared<LPE_RegState>();
	newFull.push_back(start);

	auto addState = [&](bool optional, bool repeat) {
		FSATable newTable;
		copyTable(a, newTable);

		auto end = std::make_shared<LPE_RegState>();

		connect_to_initial(start, LPE_Token(), newTable);

		if (optional)
			start->addTransition(LPE_Token(), end);

		connect_from_final(end, LPE_Token(), newTable);
		if (repeat)
			connect_to_self(LPE_Token(), newTable, false);

		remove_flags(newTable);

		start = end;

		newFull.insert(newFull.end(), newTable.begin(), newTable.end());
		newFull.push_back(end);
	};

	for (uint32 i = 0; i < min; ++i) {
		addState(false, false);
	}

	if (max == 0) { // Infinite
		addState(true, true);
	} else {
		for (uint32 i = min; i < max; ++i) {
			addState(true, false);
		}
	}

	newFull.front()->makeInitial(true);
	newFull.back()->makeFinal(true);

	mOperandStack.push(newFull);
	return true;
}

void LPE_RegExpr::connect_to_initial(const std::shared_ptr<LPE_RegState>& state,
									 const LPE_Token& token,
									 FSATable& tbl)
{
	for (auto& s : tbl) {
		if (s->isInitial())
			state->addTransition(LPE_Token(), s);
	}
}

void LPE_RegExpr::connect_from_final(const std::shared_ptr<LPE_RegState>& state,
									 const LPE_Token& token,
									 FSATable& tbl)
{
	for (auto& s : tbl) {
		if (s->isFinal())
			s->addTransition(LPE_Token(), state);
	}
}

void LPE_RegExpr::connect_to_self(const LPE_Token& token,
								  FSATable& tbl,
								  bool initialToFinal)
{
	if (initialToFinal) {
		for (auto& s : tbl) {
			if (s->isInitial()) {
				for (auto& t : tbl) {
					if (t->isFinal())
						s->addTransition(LPE_Token(), t);
				}
			}
		}
	} else {
		for (auto& s : tbl) {
			if (s->isFinal()) {
				for (auto& t : tbl) {
					if (t->isInitial())
						s->addTransition(LPE_Token(), t);
				}
			}
		}
	}
}

void LPE_RegExpr::remove_flags(FSATable& tbl, bool initialFlag, bool finalFlag)
{
	for (auto& s : tbl) {
		if (initialFlag)
			s->makeInitial(false);
		if (finalFlag)
			s->makeFinal(false);
	}
}

void LPE_RegExpr::copyTable(const FSATable& oldTable, FSATable& newTable)
{
	std::unordered_map<std::shared_ptr<LPE_RegState>, std::shared_ptr<LPE_RegState>> mapper;
	for (const auto& o : oldTable) {
		mapper[o] = std::make_shared<LPE_RegState>();
		newTable.push_back(mapper[o]);
	}

	// Reconnect transitions
	for (const auto& o : oldTable) {
		auto n = mapper[o];
		n->makeInitial(o->isInitial());
		n->makeFinal(o->isFinal());

		for (const auto& p : o->transitions()) {
			if (mapper.count(p.To)) {
				n->addTransition(p.Tokens, mapper[p.To]);
			} else {
				n->addTransition(p.Tokens, p.To);
			}
		}
	}
}

bool LPE_RegExpr::attachNFA(const LPE_RegExpr& other)
{
	mOperandStack.push(other.getNFATable());
	mInputSet.insert(other.mInputSet.begin(), other.mInputSet.end());
	return true;
}

const FSATable& LPE_RegExpr::getNFATable() const
{
	PR_ASSERT(mOperandStack.size() == 1, "Expression has to be fully proceeded!");
	return mOperandStack.top();
}

void LPE_RegExpr::convertToDFA()
{
	dfa_construct(getNFATable(), mDFA, mInputSet);

	// Optimization
	dfa_remove_deadends(mDFA);
	dfa_remove_nondistinguishables(mDFA, mInputSet);
}

void LPE_RegExpr::complementDFA()
{
	// Make final states normal and normal to final
	for (auto& s : mDFA) {
		s->makeFinal(!s->isFinal());
	}

	// Add sink to complete dfa
	auto sink = std::make_shared<LPE_RegState>();

	// TODO: Get symbol space - tokenset

	remove_flags(mDFA, false, true);
	sink->makeFinal(true);
	mDFA.push_back(sink);
}

bool LPE_RegExpr::attachDFA(const LPE_RegExpr& other)
{
	mOperandStack.push(other.getDFATable());
	mInputSet.insert(other.mInputSet.begin(), other.mInputSet.end());
	return true;
}

const FSATable& LPE_RegExpr::getDFATable() const
{
	return mDFA;
}

void LPE_RegExpr::dfa_construct(const FSATable& nfa, FSATable& dfa, const TokenSet& inputSet)
{
	dfa.clear();

	std::stack<std::shared_ptr<LPE_RegState>> unmarked;
	LPE_RegState::StateSet nfaStart;
	LPE_RegState::StateSet dfaStart;

	// Create initial set
	for (const auto& s : nfa) {
		if (s->isInitial()) {
			nfaStart.insert(s);
		}
	}
	dfa_getEpsilonClosure(nfaStart, dfaStart);

	auto dfaStartState = std::make_shared<LPE_RegState>(dfaStart);
	dfa.push_back(dfaStartState);

	unmarked.push(dfaStartState);
	while (!unmarked.empty()) {
		std::shared_ptr<LPE_RegState> ps = unmarked.top();
		unmarked.pop();

		for (const auto& token : inputSet) {
			LPE_RegState::StateSet moveRes;
			LPE_RegState::StateSet epsRes;

			dfa_move(token, ps->states(), moveRes);
			/*if (moveRes.empty())
				continue;*/

			dfa_getEpsilonClosure(moveRes, epsRes);
			/*if (epsRes.empty())
				continue;*/

			std::shared_ptr<LPE_RegState> foundS = nullptr;
			for (const auto& s : dfa) {
				if (s->states() == epsRes) {
					foundS = s;
					break;
				}
			}

			if (foundS == nullptr) {
				auto newS = std::make_shared<LPE_RegState>(epsRes);
				unmarked.push(newS);
				dfa.push_back(newS);

				ps->addTransition(token, newS);
			} else {
				ps->addTransition(token, foundS);
			}
		}
	}
}
void LPE_RegExpr::dfa_getEpsilonClosure(const LPE_RegState::StateSet& state,
										LPE_RegState::StateSet& res)
{
	res.clear();
	res = state;

	std::stack<std::shared_ptr<LPE_RegState>> todoStack;
	for (const auto& s : state)
		todoStack.push(s);

	while (!todoStack.empty()) {
		std::shared_ptr<LPE_RegState> s = todoStack.top();
		todoStack.pop();

		for (const auto& p : s->transitions()) {
			if (!p.hasToken(LPE_Token())) // Only empty closures
				continue;

			if (res.find(p.To) == res.end()) {
				res.insert(p.To);
				todoStack.push(p.To);
			}
		}
	}
}

void LPE_RegExpr::dfa_move(const LPE_Token& token,
						   const LPE_RegState::StateSet& state,
						   LPE_RegState::StateSet& res)
{
	res.clear();

	for (const auto& s : state) {
		for (const auto& p : s->transitions()) {
			if (!p.hasToken(token))
				continue;

			res.insert(p.To);
		}
	}
}

void LPE_RegExpr::dfa_remove_deadends(FSATable& dfa)
{
	LPE_RegState::StateSet deadends;

	for (const auto& s : dfa) {
		if (s->isDeadEnd())
			deadends.insert(s);
	}

	for (auto it = dfa.begin(); it != dfa.end();) {
		if ((*it)->isDeadEnd()) {
			it = dfa.erase(it);
		} else {
			for (const auto& t : deadends)
				(*it)->removeTransitionsTo(t);
			++it;
		}
	}
}

void LPE_RegExpr::dfa_remove_nondistinguishables(FSATable& dfa, const TokenSet& inputSet)
{
	// Brzozowski's algorithm

	// Reverse edges
	FSATable reverseNFA;
	dfa_reverse(dfa, reverseNFA);

	// Construct DFA
	FSATable reverseDFA;
	dfa_construct(reverseNFA, reverseDFA, inputSet);
	dfa_remove_deadends(reverseDFA);

	// Reverse edges again
	FSATable nfa;
	dfa_reverse(reverseDFA, nfa);

	// Construct DFA again
	dfa_construct(nfa, dfa, inputSet);
	dfa_remove_deadends(dfa);
}

void LPE_RegExpr::dfa_reverse(const FSATable& dfa, FSATable& reverseNFA)
{
	reverseNFA.clear();

	std::unordered_map<std::shared_ptr<LPE_RegState>, std::shared_ptr<LPE_RegState>> mapper;
	for (const auto& s : dfa) {
		auto ns = std::make_shared<LPE_RegState>();

		// Flip
		ns->makeInitial(s->isFinal());
		ns->makeFinal(s->isInitial());

		mapper[s] = ns;
		reverseNFA.push_front(ns);
	}

	for (const auto& s : dfa) {
		for (const auto& t : s->transitions()) {
			mapper[t.To]->addTransition(t.Tokens, mapper[s]);
		}
	}
}

// Debug
std::string LPE_RegExpr::dumpTable(const FSATable& tbl)
{
	std::stringstream stream;

	for (const auto& s : tbl) {
		stream << "{";
		for (const auto& p : s->transitions()) {
			stream << "[";
			for (const auto& k : p.Tokens)
				stream << "<" << k.Type << "," << k.Event << ">";
			stream << "]";
		}
		stream << "}(" << s->transitionCount() << ")";
		if (s->isInitial())
			stream << "I";
		if (s->isFinal())
			stream << "F";
		stream << std::endl;
	}

	return stream.str();
}

std::string LPE_RegExpr::dumpTableToDot(const FSATable& tbl)
{
	std::stringstream stream;
	stream << "digraph {" << std::endl;

	std::unordered_map<std::shared_ptr<LPE_RegState>, uint32> ids;
	uint32 idCounter = 0;
	for (const auto& s : tbl) {
		if (!ids.count(s)) {
			ids[s] = idCounter;
			++idCounter;
		}

		if (s->isFinal() && s->isInitial()) {
			stream << ids[s] << "[shape=tripleoctagon];" << std::endl;
		} else if (s->isFinal()) {
			stream << ids[s] << "[shape=doublecircle];" << std::endl;
		} else if (s->isInitial()) {
			stream << ids[s] << "[shape=Mcircle];" << std::endl;
		}
	}

	for (const auto& s : tbl) {
		uint32 currentID = ids[s];
		for (const auto& t : s->transitions()) {
			uint32 childID = ids[t.To];

			stream << currentID << "\t->\t" << childID;
			stream << "\t[label=\"";

			for (const auto& k : t.Tokens) {
				if (k.isEmpty()) {
					stream << "e,";
				} else if (k.Label.empty()) {
					stream << "<" << k.Type << "," << k.Event << ">,";
				} else {
					stream << "<" << k.Type << "," << k.Event << ",'" << k.Label << "'>,";
				}
			}
			stream << "\"];" << std::endl;
		}
	}

	stream << "}";
	return stream.str();
}

void LPE_RegExpr::saveTableToDot(const std::string& filename, const FSATable& tbl)
{
	std::ofstream dotFile;
	dotFile.open(filename);
	dotFile << LPE_RegExpr::dumpTableToDot(tbl);
	dotFile.close();
}
} // namespace PR