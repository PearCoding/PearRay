#pragma once

#include "LPE_RegExpr.h"
#include "LightPath.h"
#include "LightPathView.h"

namespace PR {
namespace LPE {
class RegExpr;
class Automaton {
public:
	Automaton()	 = default;
	~Automaton() = default;

	bool build(const std::shared_ptr<RegExpr>& expr);

	template <typename Path>
	inline bool match(const Path& path) const
	{
		bool success;

		size_t currentState = mStartingState;
		for (size_t i = 0; i < path.currentSize(); ++i) {
			size_t ns = nextState(currentState, path.token(i), success);

			//PR_LOG(L_INFO) << currentState << " -> <" << path.token(i).Type << "," << path.token(i).Event << "> -> " << ns << std::endl;
			currentState = ns;
			if (!success)
				return false;
		}

		return mSB_IsFinal[currentState];
	}

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