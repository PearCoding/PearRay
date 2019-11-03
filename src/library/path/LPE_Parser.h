#pragma once

#include "PR_Config.h"

namespace PR {
namespace LPE {
class RegExpr;
class Parser {
public:
	Parser(const std::string& str);
	~Parser() = default;

	std::shared_ptr<RegExpr> parse();

private:
	char current() const;
	char peek() const;
	bool accept(char c);

	inline bool isEOS() const { return current() == -1; }

	void gr_fullexpr(RegExpr& expr);
	void gr_expr(RegExpr& expr);
	void gr_term(RegExpr& expr);
	void gr_group(RegExpr& expr);
	void gr_orgroup(RegExpr& expr);
	void gr_token(RegExpr& expr);
	void gr_op(RegExpr& expr);

	std::string gr_string();
	uint32 gr_integer();

	bool mError;
	size_t mPosition;
	std::string mString;
};
} // namespace LPE
} // namespace PR