#include "LPE_Parser.h"
#include "LPE_RegExpr.h"
#include "Logger.h"

#include <cctype>

namespace PR {
namespace LPE {
Parser::Parser(const std::string& str)
	: mError(false)
	, mPosition(0)
	, mString(str)
{
}

std::shared_ptr<RegExpr> Parser::parse()
{
	auto r = std::make_shared<RegExpr>();
	gr_fullexpr(*r);

	if (mError || !r->isValid()) {
		return nullptr;
	} else {
		r->convertToDFA();
		/*PR_LOG(L_INFO) << RegExpr::dumpTable(r->getNFATable()) << std::endl;
		PR_LOG(L_INFO) << RegExpr::dumpTable(r->getDFATable()) << std::endl;

		RegExpr::saveTableToDot("nfa.dot", r->getNFATable());
		RegExpr::saveTableToDot("dfa.dot", r->getDFATable());*/
		return r;
	}
}

char Parser::current() const
{
	if (mPosition < mString.size())
		return mString.at(mPosition);
	else
		return -1;
}

char Parser::peek() const
{
	if (mPosition + 1 < mString.size())
		return mString.at(mPosition + 1);
	else
		return -1;
}

bool Parser::accept(char c)
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

void Parser::gr_fullexpr(RegExpr& expr)
{
	accept('C');
	expr.push(Token('C', '.'));

	RegExpr other;
	gr_expr(other);

	if (other.isValid()) {
		expr.attachNFA(other);
		expr.doConcat();
	}
}

void Parser::gr_expr(RegExpr& expr)
{
	gr_term(expr);

	while (!isEOS() && !mError && current() != ')') {
		gr_term(expr);
		expr.doConcat();
	}
}

void Parser::gr_term(RegExpr& expr)
{
	if (current() == 'D'
		|| current() == 'S'
		|| current() == 'E'
		|| current() == 'L'
		|| current() == 'B'
		|| current() == 'R'
		|| current() == 'T'
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
		PR_LOG(L_ERROR) << "LPE Syntax Error[" << mPosition << "]: Expected one valid token or a group but got '" << current() << "' instead" << std::endl;
		mError = true;
	}
}

void Parser::gr_group(RegExpr& expr)
{
	accept('(');
	RegExpr other;
	gr_expr(other);
	accept(')');

	//PR_LOG(L_INFO) << "[" << RegExpr::dumpTable(other.getNFATable()) << "]" << std::endl;

	if (other.isValid())
		expr.attachNFA(other);
}

void Parser::gr_orgroup(RegExpr& expr)
{
	bool negate = false;
	accept('[');
	if (current() == '^') {
		accept('^');
		negate = true;
	}

	RegExpr other;
	gr_term(other);
	while (!isEOS() && !mError && current() != ']') {
		gr_term(other);
		other.doUnion();
	}

	accept(']');

	if (!other.isValid())
		return;

	if (negate) {
		PR_LOG(L_ERROR) << "Negation in union groups currently not supported!" << std::endl;
		mError = true;
		/*other.convertToDFA();
		other.complementDFA();
		expr.attachDFA(other);*/
	} else {
		//PR_LOG(L_INFO) << "O[" << RegExpr::dumpTable(other.getNFATable()) << "]" << std::endl;
		expr.attachNFA(other);
	}
}

void Parser::gr_token(RegExpr& expr)
{
	if (current() == 'D') {
		accept('D');
		expr.push(Token('.', 'D'));
	} else if (current() == 'S') {
		accept('S');
		expr.push(Token('.', 'S'));
	} else if (current() == 'E') {
		accept('E');
		expr.push(Token('E', '.'));
	} else if (current() == 'L') {
		accept('L');
		expr.push(Token('L', '.'));
	} else if (current() == 'B') {
		accept('B');
		expr.push(Token('B', '.'));
	} else if (current() == '.') {
		accept('.');
		expr.push(Token('.', '.'));
	} else if (current() == 'R') {
		accept('R');
		expr.push(Token('R', '.'));
	} else if (current() == 'T') {
		accept('T');
		expr.push(Token('T', '.'));
	}else if (current() == '<') {
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

		// A colon between entries is optional
		if (current() == ',')
			accept(',');

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
		if (current() == '"' || current() == ',') {
			if (current() == ',')
				accept(',');
			lbl = gr_string();
		}
		accept('>');

		expr.push(Token(t, e, lbl));
	} else {
		PR_LOG(L_ERROR) << "LPE Syntax Error[" << mPosition << "]: Unknown token " << current() << std::endl;
		mError = true;
	}
}

void Parser::gr_op(RegExpr& expr)
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

std::string Parser::gr_string()
{
	std::string str = "";
	accept('"');
	while (!isEOS() && !mError && current() != '"') {
		str += current();
		accept(current());
	}
	accept('"');
	return str;
}

uint32 Parser::gr_integer()
{
	std::string number = "";
	while (!isEOS() && !mError && std::isdigit(current())) {
		number += current();
		accept(current());
	}
	return std::stoi(number);
}
} // namespace LPE
} // namespace PR