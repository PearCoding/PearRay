#pragma once

#include "PR_Config.h"

namespace PR {
namespace LPE {
class Automaton;
}

class LightPath;
class LightPathView;

class PR_LIB_CORE LightPathExpression {
public:
	LightPathExpression(const std::string& str);
	~LightPathExpression();

	bool parseString(const std::string& str);
	bool match(const LightPath& path) const;
	bool match(const LightPathView& path) const;

	inline bool isValid() const { return mAutomaton != nullptr; }

	std::string dumpTable() const;

	static std::string generateTableString(const std::string& expr); 
	static std::string generateDotString(const std::string& expr); 
private:
	std::shared_ptr<LPE::Automaton> mAutomaton;
};
} // namespace PR