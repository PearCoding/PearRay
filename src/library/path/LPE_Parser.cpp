#include "LPE_Parser.h"
#include "LPE_RegExpr.h"
#include "Logger.h"

#include <cctype>

namespace PR {
LPE_Parser::LPE_Parser(const std::string& str)
	: mError(false)
	, mPosition(0)
	, mString(str)
{
}

std::shared_ptr<LPE_RegExpr> LPE_Parser::parse()
{
	auto r = std::make_shared<LPE_RegExpr>();
	gr_fullexpr(*r);

	if (mError || !r->isValid()) {
		return nullptr;
	} else {
		r->convertToDFA();
		/*PR_LOG(L_INFO) << LPE_RegExpr::dumpTable(r->getNFATable()) << std::endl;
		PR_LOG(L_INFO) << LPE_RegExpr::dumpTable(r->getDFATable()) << std::endl;

		LPE_RegExpr::saveTableToDot("nfa.dot", r->getNFATable());
		LPE_RegExpr::saveTableToDot("dfa.dot", r->getDFATable());*/
		return r;
	}
}

char LPE_Parser::current() const
{
	if (mPosition < mString.size())
		return mString.at(mPosition);
	else
		return -1;
}

char LPE_Parser::peek() const
{
	if (mPosition + 1 < mString.size())
		return mString.at(mPosition + 1);
	else
		return -1;
}

bool LPE_Parser::accept(char c)
{
	if (current() != c) {
		PR_LOG(L_ERROR) << "LPE Syntax Error[" << mPosition << "]: Expected '" << c << "' but got '" << current() << "' instead" << std::endl;

		mError = true;
		mPosition++;
		return false;
	} else {
		mPosition++;
		return true;
	}
}

void LPE_Parser::gr_fullexpr(LPE_RegExpr& expr)
{
	accept('C');
	expr.push(LPE_Token('C', '.'));

	LPE_RegExpr other;
	gr_expr(other);

	if (other.isValid()) {
		expr.attachNFA(other);
		expr.doConcat();
	}
}

void LPE_Parser::gr_expr(LPE_RegExpr& expr)
{
	gr_term(expr);

	while (!isEOS() && current() != ')') {
		gr_term(expr);
		expr.doConcat();
	}
}

void LPE_Parser::gr_term(LPE_RegExpr& expr)
{
	if (current() == 'D'
		|| current() == 'S'
		|| current() == 'E'
		|| current() == 'L'
		|| current() == 'B'
		|| current() == '.'
		|| current() == '<') {
		gr_token(expr);
		gr_op(expr);
	} else if (current() == '(') {
		gr_group(expr);
		gr_op(expr);
	} else if (current() == '[') {
		gr_orgroup(expr);
		gr_op(expr);
	} else {
		mError = true;
		// TODO: Error
	}
}

void LPE_Parser::gr_group(LPE_RegExpr& expr)
{
	accept('(');
	LPE_RegExpr other;
	gr_expr(other);
	accept(')');

	//PR_LOG(L_INFO) << "[" << LPE_RegExpr::dumpTable(other.getNFATable()) << "]" << std::endl;

	if (other.isValid())
		expr.attachNFA(other);
}

void LPE_Parser::gr_orgroup(LPE_RegExpr& expr)
{
	bool negate = false;
	accept('[');
	if (current() == '^') {
		accept('^');
		negate = true;
	}

	LPE_RegExpr other;
	gr_term(other);
	while (!isEOS() && current() != ']') {
		gr_term(other);
		other.doUnion();
	}

	accept(']');

	if(!other.isValid())
		return;

	if (negate) {
		PR_LOG(L_ERROR) << "Negation in union groups currently not supported!" << std::endl;
		mError = true;
		/*other.convertToDFA();
		other.complementDFA();
		expr.attachDFA(other);*/
	} else {
		//PR_LOG(L_INFO) << "O[" << LPE_RegExpr::dumpTable(other.getNFATable()) << "]" << std::endl;
		expr.attachNFA(other);
	}
}

void LPE_Parser::gr_token(LPE_RegExpr& expr)
{
	if (current() == 'D') {
		accept('D');
		expr.push(LPE_Token('.', 'D'));
	} else if (current() == 'S') {
		accept('S');
		expr.push(LPE_Token('.', 'S'));
	} else if (current() == 'E') {
		accept('E');
		expr.push(LPE_Token('E', '.'));
	} else if (current() == 'L') {
		accept('L');
		expr.push(LPE_Token('L', '.'));
	} else if (current() == 'B') {
		accept('B');
		expr.push(LPE_Token('B', '.'));
	} else if (current() == '.') {
		accept('.');
		expr.push(LPE_Token('.', '.'));
	} else if (current() == '<') {
		accept('<');
		char t; // Type
		if (current() == 'E') {
			accept('E');
			t = 'E';
		} else if (current() == 'L') {
			accept('L');
			t = 'L';
		} else if (current() == 'B') {
			accept('B');
			t = 'B';
		} else if (current() == 'R') {
			accept('R');
			t = 'R';
		} else if (current() == 'T') {
			accept('T');
			t = 'T';
		} else if (current() == '.') {
			accept('.');
			t = '.';
		} else {
			PR_LOG(L_ERROR) << "LPE Syntax Error[" << mPosition << "]: Expected one scattering type but got '" << current() << "' instead" << std::endl;
			mError = true;
			return;
		}

		char e; // Event
		if (current() == 'D') {
			accept('D');
			e = 'D';
		} else if (current() == 'S') {
			accept('S');
			e = 'S';
		} else if (current() == '.') {
			accept('.');
			e = '.';
		} else {
			PR_LOG(L_ERROR) << "LPE Syntax Error[" << mPosition << "]: Expected one scattering event but got '" << current() << "' instead" << std::endl;
			mError = true;
			return;
		}

		std::string lbl = "";
		if (current() == '"') {
			lbl = gr_string();
		}
		accept('>');

		expr.push(LPE_Token(t, e, lbl));
	}
}

void LPE_Parser::gr_op(LPE_RegExpr& expr)
{
	if (current() == '*') {
		accept('*');
		expr.repeatLast(0, 0);
	} else if (current() == '+') {
		accept('+');
		expr.repeatLast(1, 0);
	} else if (current() == '?') {
		accept('?');
		expr.repeatLast(0, 1);
	} else if (current() == '{') { // {n} or {start,end}
		accept('{');
		uint32 n = gr_integer();
		uint32 l = n;
		if (current() == ',') {
			accept(',');
			l = gr_integer();
		}
		accept('}');

		if (l < n) {
			PR_LOG(L_ERROR) << "LPE Syntax Error[" << mPosition << "]: Maximum " << l << " less then minimum " << n << std::endl;
			mError = true;
			return;
		}
		expr.repeatLast(n, l);
	}
}

std::string LPE_Parser::gr_string()
{
	std::string str = "";
	accept('"');
	while (!isEOS() && current() != '"') {
		str += current();
		accept(current());
	}
	accept('"');
	return str;
}

uint32 LPE_Parser::gr_integer()
{
	std::string number = "";
	while (!isEOS() && std::isdigit(current())) {
		number += current();
		accept(current());
	}
	return std::stoi(number);
}
} // namespace PR