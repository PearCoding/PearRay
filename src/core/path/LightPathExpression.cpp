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
		bool b	   = mAutomaton->build(reg);
		return b;
	} else {
		return false;
	}
}

bool LightPathExpression::match(const LightPath& path) const
{
	return mAutomaton->match(path);
}

bool LightPathExpression::match(const LightPathView& path) const
{
	return mAutomaton->match(path);
}

std::string LightPathExpression::dumpTable() const
{
	return mAutomaton->dumpTable();
}

std::string LightPathExpression::generateTableString(const std::string& expr)
{
	LightPathExpression obj(expr);
	return obj.isValid() ? obj.dumpTable() : "";
}

std::string LightPathExpression::generateDotString(const std::string& expr)
{
	LPE::Parser parser(expr);
	auto reg = parser.parse();

	return reg ? LPE::RegExpr::dumpTableToDot(reg->getDFATable()) : "";
}
} // namespace PR