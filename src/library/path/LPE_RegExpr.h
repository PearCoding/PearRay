#pragma once

#include "LPE_RegState.h"
#include <deque>
#include <set>
#include <stack>
#include <unordered_set>

namespace PR {
typedef std::deque<std::shared_ptr<LPE_RegState>> FSATable;

class LPE_RegExpr {
public:
	LPE_RegExpr();

	void push(const LPE_Token& token);
	bool pop(FSATable& table);

	bool doConcat();
	bool doUnion();

	// max == 0 -> infinite
	bool repeatLast(uint32 min, uint32 max);

	bool attachNFA(const LPE_RegExpr& other);
	const FSATable& getNFATable() const;

	void convertToDFA();
	bool attachDFA(const LPE_RegExpr& other);
	const FSATable& getDFATable() const;

	void complementDFA();

	static std::string dumpTable(const FSATable& tbl);
	static std::string dumpTableToDot(const FSATable& tbl);
	static void saveTableToDot(const std::string& filename, const FSATable& tbl);

	inline bool isValid() const { return mOperandStack.size() == 1; }

private:
	static void connect_to_initial(const std::shared_ptr<LPE_RegState>& state,
								   const LPE_Token& token,
								   FSATable& tbl);
	static void connect_from_final(const std::shared_ptr<LPE_RegState>& state,
								   const LPE_Token& token,
								   FSATable& tbl);
	static void connect_to_self(const LPE_Token& token,
								FSATable& tbl,
								bool initialToFinal = false);
	static void remove_flags(FSATable& tbl, bool initialFlag = true, bool finalFlag = true);

	// DFA creation
	static void dfa_construct(const FSATable& nfa, FSATable& dfa, const TokenSet& inputSet);
	static void dfa_getEpsilonClosure(const LPE_RegState::StateSet& state,
									  LPE_RegState::StateSet& res);

	static void dfa_move(const LPE_Token& token,
						 const LPE_RegState::StateSet& state,
						 LPE_RegState::StateSet& res);

	static void dfa_remove_deadends(FSATable& dfa);
	static void dfa_remove_nondistinguishables(FSATable& dfa, const TokenSet& inputSet);
	static void dfa_reverse(const FSATable& dfa, FSATable& reverseNFA);

	void copyTable(const FSATable& oldTable, FSATable& newTable);

	// Build process
	std::stack<FSATable> mOperandStack;
	TokenSet mInputSet;

	FSATable mDFA;
};
} // namespace PR