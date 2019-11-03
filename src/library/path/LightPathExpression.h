#pragma once

#include "PR_Config.h"

namespace PR {
namespace LPE {
class Automaton;
}

class LightPath;
class PR_LIB LightPathExpression {
public:
	LightPathExpression(const std::string& str);
	~LightPathExpression();

	bool parseString(const std::string& str);
	bool match(const LightPath& path) const;

	inline bool isValid() const { return mAutomaton != nullptr; }

private:
	std::shared_ptr<LPE::Automaton> mAutomaton;
};
} // namespace PR