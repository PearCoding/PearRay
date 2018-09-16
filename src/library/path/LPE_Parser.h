#pragma once

#include "PR_Config.h"

namespace PR {
class LPE_RegExpr;
class PR_LIB LPE_Parser
{
public:
	LPE_Parser(const std::string& str);
	~LPE_Parser() = default;

	std::shared_ptr<LPE_RegExpr> parse();
private:
	char current() const;
	char peek() const;
	bool accept(char c);

	inline bool isEOS() const { return current() == -1; }

	void gr_fullexpr(LPE_RegExpr& expr);
	void gr_expr(LPE_RegExpr& expr);
	void gr_term(LPE_RegExpr& expr);
	void gr_group(LPE_RegExpr& expr);
	void gr_orgroup(LPE_RegExpr& expr);
	void gr_token(LPE_RegExpr& expr);
	void gr_op(LPE_RegExpr& expr);

	std::string gr_string();
	uint32 gr_integer();

	bool mError;
	size_t mPosition;
	std::string mString;
};
} // namespace PR