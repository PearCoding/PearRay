#pragma once

#include "LPE_RegExpr.h"
#include "LightPath.h"

namespace PR {
namespace LPE {
class RegExpr;
class Automaton {
public:
	Automaton()  = default;
	~Automaton() = default;

	bool build(const std::shared_ptr<RegExpr>& expr);
	bool match(const LightPath& path) const;

	std::string dumpTable() const;

private:
	size_t nextState(size_t currentState, const LightPathToken& token, bool& success) const;

	// SoA
	// {Label Block}
	std::vector<size_t> mLB_LabelIndices;
	std::vector<size_t> mLB_NextStates;

	// {Inside Block} = States * Type_Count * Event_Count
	std::vector<bool> mIB_Allowed;
	std::vector<size_t> mIB_LabelBlockStart;
	std::vector<size_t> mIB_LabelBlockSize;
	std::vector<size_t> mIB_EmptyNextState;

	// {State Block} = States
	std::vector<bool> mSB_IsFinal;

	size_t mStartingState;
};
} // namespace LPE
} // namespace PR