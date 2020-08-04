#pragma once

#include "LPE_RegState.h"
#include <deque>
#include <filesystem>
#include <set>
#include <stack>
#include <unordered_set>

namespace PR {
namespace LPE {
typedef std::deque<std::shared_ptr<RegState>> FSATable;

class RegExpr {
public:
	RegExpr();

	void push(const Token& token);
	bool pop(FSATable& table);

	bool doConcat();
	bool doUnion();

	// max == 0 -> infinite
	bool repeatLast(uint32 min, uint32 max);

	bool attachNFA(const RegExpr& other);
	const FSATable& getNFATable() const;

	void convertToDFA();
	bool attachDFA(const RegExpr& other);
	const FSATable& getDFATable() const;

	void complementDFA();

	static std::string dumpTable(const FSATable& tbl);
	static std::string dumpTableToDot(const FSATable& tbl);
	static void saveTableToDot(const std::filesystem::path& filename, const FSATable& tbl);

	inline bool isValid() const { return mOperandStack.size() == 1; }

private:
	static void connect_to_initial(const std::shared_ptr<RegState>& state,
								   const Token& token,
								   FSATable& tbl);
	static void connect_from_final(const std::shared_ptr<RegState>& state,
								   const Token& token,
								   FSATable& tbl);
	static void connect_to_self(const Token& token,
								FSATable& tbl,
								bool initialToFinal = false);
	static void remove_flags(FSATable& tbl, bool initialFlag = true, bool finalFlag = true);

	// DFA creation
	static void dfa_construct(const FSATable& nfa, FSATable& dfa, const TokenSet& inputSet);
	static void dfa_getEpsilonClosure(const RegState::StateSet& state,
									  RegState::StateSet& res);

	static void dfa_move(const Token& token,
						 const RegState::StateSet& state,
						 RegState::StateSet& res);

	static void dfa_remove_deadends(FSATable& dfa);
	static void dfa_remove_nondistinguishables(FSATable& dfa, const TokenSet& inputSet);
	static void dfa_reverse(const FSATable& dfa, FSATable& reverseNFA);

	void copyTable(const FSATable& oldTable, FSATable& newTable);

	// Build process
	std::stack<FSATable> mOperandStack;
	TokenSet mInputSet;

	FSATable mDFA;
};
} // namespace LPE
} // namespace PR