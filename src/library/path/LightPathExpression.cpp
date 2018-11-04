#include "LightPathExpression.h"
#include "LPE_Automaton.h"
#include "LPE_Parser.h"
#include "Logger.h"

namespace PR {
LightPathExpression::LightPathExpression(const std::string& str)
{
	parseString(str);
}

LightPathExpression::~LightPathExpression()
{
}

bool LightPathExpression::parseString(const std::string& str)
{
	mAutomaton.reset();

	LPE::Parser parser(str);
	auto reg = parser.parse();

	if (reg) {
		mAutomaton = std::make_shared<LPE::Automaton>();
		bool b	 = mAutomaton->build(reg);
		//PR_LOG(L_INFO) << mAutomaton->dumpTable() << std::endl;
		return b;
	} else {
		return false;
	}
}

bool LightPathExpression::match(const LightPath& path) const
{
	return mAutomaton->match(path);
}
} // namespace PR