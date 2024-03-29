#include "LPE_Automaton.h"
#include "LightPathManager.h"

#include "Logger.h"

namespace PR {
namespace LPE {
#define INSIDE_BLOCK_INDEX(s, t, e) (((uint32)e) * (uint32)ScatteringType::_COUNT * mSB_IsFinal.size() + ((uint32)t) * mSB_IsFinal.size() + (s))

constexpr size_t NO_EMPTY = std::numeric_limits<size_t>::max();

bool Automaton::build(const std::shared_ptr<RegExpr>& expr)
{
	auto table = expr->getDFATable();

	// Assign ids
	std::unordered_map<std::shared_ptr<RegState>, uint32> ids;
	uint32 idCounter = 0;
	for (const auto& s : table) {
		ids[s] = idCounter;
		++idCounter;
	}
	mSB_IsFinal.resize(idCounter, false);

	mIB_Allowed.resize(idCounter * (uint32)ScatteringType::_COUNT * (uint32)ScatteringEvent::_COUNT, false);
	mIB_EmptyNextState.resize(idCounter * (uint32)ScatteringType::_COUNT * (uint32)ScatteringEvent::_COUNT, NO_EMPTY);
	mIB_LabelBlockSize.resize(idCounter * (uint32)ScatteringType::_COUNT * (uint32)ScatteringEvent::_COUNT, 0);
	mIB_LabelBlockStart.resize(idCounter * (uint32)ScatteringType::_COUNT * (uint32)ScatteringEvent::_COUNT, 0);

	mStartingState = 0;
	for (const auto& s : table) {
		uint32 id = ids[s];

		if (s->isInitial())
			mStartingState = id;

		mSB_IsFinal[id] = s->isFinal();

		for (uint32 t = 0; t < (uint32)ScatteringType::_COUNT; ++t) {
			for (uint32 e = 0; e < (uint32)ScatteringEvent::_COUNT; ++e) {
				const size_t index	 = INSIDE_BLOCK_INDEX(id, t, e);
				const size_t lblAddr = mLB_LabelIndices.size();
				bool foundLabeled	 = false;
				bool foundEmpty		 = false;
				for (const auto& trans : s->transitions()) {
					uint32 nextID = ids[trans.To];

					for (const auto& k : trans.Tokens) {
						if (!k.match((ScatteringType)t, (ScatteringEvent)e)
							|| k.Label.empty())
							continue;

						mLB_LabelIndices.push_back(LightPathManager::getLabelIndex(k.Label));
						mLB_NextStates.push_back(nextID);

						foundLabeled = true;
					}

					for (const auto& k : trans.Tokens) {
						if (!k.match((ScatteringType)t, (ScatteringEvent)e)
							|| !k.Label.empty())
							continue;

						if (foundEmpty)
							continue;

						mIB_EmptyNextState[index] = nextID;
						foundEmpty				  = true;
						break;
					}
				}

				if (foundLabeled || foundEmpty) {
					PR_ASSERT(!mIB_Allowed[index], "Invalid DFA");

					mIB_Allowed[index]		   = true;
					mIB_LabelBlockStart[index] = lblAddr;
					mIB_LabelBlockStart[index] = mLB_LabelIndices.size() - lblAddr;
				}
			}
		}
	}

	return true;
}

size_t Automaton::nextState(size_t currentState, const LightPathToken& token, bool& success) const
{
	success		 = false;
	size_t index = INSIDE_BLOCK_INDEX(currentState, token.Type, token.Event);

	if (mIB_Allowed[index]) {
		if (token.LabelIndex == 0 && mIB_EmptyNextState[index] != NO_EMPTY) {
			success = true;
			return mIB_EmptyNextState[index];
		} else if (token.LabelIndex != 0) {
			const size_t saddr = mIB_LabelBlockStart[index];
			for (size_t i = 0; mIB_LabelBlockSize[index] != 0; ++i) {
				if (mLB_LabelIndices[saddr + i] == token.LabelIndex) {
					success = true;
					return mLB_NextStates[saddr + i];
				}
			}

			if (mIB_EmptyNextState[index] != NO_EMPTY) {
				success = true;
				return mIB_EmptyNextState[index];
			}
		}
	}
	return currentState;
}

// Debug
const char _st_char[(uint32)ScatteringType::_COUNT]	 = { 'C', 'E', 'T', 'R', 'B' };
const char _se_char[(uint32)ScatteringEvent::_COUNT] = { 'D', 'S', 'N' };
std::string Automaton::dumpTable() const
{
	std::stringstream stream;
	for (uint32 i = 0; i < mSB_IsFinal.size(); ++i) {
		stream << i;
		if (i == mStartingState)
			stream << " S";
		if (mSB_IsFinal[i])
			stream << " F";
		stream << std::endl;

		for (uint32 t = 0; t < (uint32)ScatteringType::_COUNT; ++t) {
			for (uint32 e = 0; e < (uint32)ScatteringEvent::_COUNT; ++e) {
				stream << "  " << _st_char[t] << ", " << _se_char[e];

				size_t index = INSIDE_BLOCK_INDEX(i, t, e);
				if (!mIB_Allowed[index]) {
					stream << " NOT ALLOWED" << std::endl;
				} else {
					if (mIB_EmptyNextState[index] != NO_EMPTY) {
						stream << " -> " << mIB_EmptyNextState[index];
					}
					stream << std::endl;

					size_t saddr = mIB_LabelBlockStart[index];
					for (uint32 l = 0; mIB_LabelBlockSize[index] != 0; ++l) {
						stream << "  - " << mLB_LabelIndices[saddr + l] << " -> " << mLB_NextStates[saddr + l] << std::endl;
					}
				}
			}
		}
	}

	return stream.str();
}

} // namespace LPE
} // namespace PR