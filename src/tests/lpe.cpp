#include "path/LightPath.h"
#include "path/LightPathExpression.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(LPE)
PR_TEST("[CD*L]")
{
	LightPathExpression expr("CD*L");
	PR_CHECK_TRUE(expr.isValid());

	LightPath path; // CDE
	path.addToken(LightPathToken::Camera());
	path.addToken(LightPathToken(ST_REFLECTION, SE_DIFFUSE));
	path.addToken(LightPathToken(ST_EMISSIVE, SE_DIFFUSE));
	PR_CHECK_TRUE(expr.match(path));

	LightPath path2; // CE
	path2.addToken(LightPathToken::Camera());
	path2.addToken(LightPathToken(ST_EMISSIVE, SE_DIFFUSE));
	PR_CHECK_TRUE(expr.match(path2));

	LightPath path3; // CSE
	path3.addToken(LightPathToken::Camera());
	path3.addToken(LightPathToken(ST_REFLECTION, SE_SPECULAR));
	path3.addToken(LightPathToken(ST_EMISSIVE, SE_DIFFUSE));
	PR_CHECK_FALSE(expr.match(path3));
}
PR_TEST("[RD*L] (Invalid)")
{
	LightPathExpression expr("RD*L");
	PR_CHECK_FALSE(expr.isValid());
}
PR_TEST("[C(DS)+D?E]")
{
	LightPathExpression expr("C(DS)+D?E");
	PR_CHECK_TRUE(expr.isValid());

	LightPath path; // CDSE
	path.addToken(LightPathToken::Camera());
	path.addToken(LightPathToken(ST_REFLECTION, SE_DIFFUSE));
	path.addToken(LightPathToken(ST_REFRACTION, SE_SPECULAR));
	path.addToken(LightPathToken(ST_EMISSIVE, SE_DIFFUSE));
	PR_CHECK_TRUE(expr.match(path));

	LightPath path2; // CDSDSDE
	path2.addToken(LightPathToken::Camera());
	path2.addToken(LightPathToken(ST_REFLECTION, SE_DIFFUSE));
	path2.addToken(LightPathToken(ST_REFRACTION, SE_SPECULAR));
	path2.addToken(LightPathToken(ST_REFLECTION, SE_DIFFUSE));
	path2.addToken(LightPathToken(ST_REFRACTION, SE_SPECULAR));
	path2.addToken(LightPathToken(ST_REFLECTION, SE_DIFFUSE));
	path2.addToken(LightPathToken(ST_EMISSIVE, SE_DIFFUSE));
	PR_CHECK_TRUE(expr.match(path2));

	LightPath path3; // CSE
	path3.addToken(LightPathToken::Camera());
	path3.addToken(LightPathToken(ST_REFLECTION, SE_SPECULAR));
	path3.addToken(LightPathToken(ST_EMISSIVE, SE_DIFFUSE));
	PR_CHECK_FALSE(expr.match(path3));
}
PR_TEST("[C[DS]+D?B]")
{
	LightPathExpression expr("C[DS]+D?B");
	PR_CHECK_TRUE(expr.isValid());

	LightPath path; // CDSB
	path.addToken(LightPathToken::Camera());
	path.addToken(LightPathToken(ST_REFLECTION, SE_DIFFUSE));
	path.addToken(LightPathToken(ST_REFRACTION, SE_SPECULAR));
	path.addToken(LightPathToken::Background());
	PR_CHECK_TRUE(expr.match(path));

	LightPath path2; // CDSDDB
	path2.addToken(LightPathToken::Camera());
	path2.addToken(LightPathToken(ST_REFLECTION, SE_DIFFUSE));
	path2.addToken(LightPathToken(ST_REFRACTION, SE_SPECULAR));
	path2.addToken(LightPathToken(ST_REFLECTION, SE_DIFFUSE));
	path2.addToken(LightPathToken(ST_REFLECTION, SE_DIFFUSE));
	path2.addToken(LightPathToken::Background());
	PR_CHECK_TRUE(expr.match(path2));

	LightPath path3; // CEB
	path3.addToken(LightPathToken::Camera());
	path3.addToken(LightPathToken(ST_EMISSIVE, SE_SPECULAR));
	path3.addToken(LightPathToken::Background());
	PR_CHECK_FALSE(expr.match(path3));
}
PR_TEST("Add/Pop")
{
	LightPath path; // CDSB
	PR_CHECK_TRUE(path.isEmpty());

	path.addToken(LightPathToken::Camera());
	path.addToken(LightPathToken(ST_REFLECTION, SE_DIFFUSE));
	path.addToken(LightPathToken(ST_REFRACTION, SE_SPECULAR));
	path.addToken(LightPathToken::Background());

	PR_CHECK_EQ(path.currentSize(), 4);
	PR_CHECK_EQ(path.containerSize(), 4);
	PR_CHECK_FALSE(path.isEmpty());

	path.popToken(2);
	PR_CHECK_EQ(path.currentSize(), 2);
	PR_CHECK_EQ(path.containerSize(), 4);
	PR_CHECK_FALSE(path.isEmpty());

	path.popToken(2);
	PR_CHECK_EQ(path.currentSize(), 0);
	PR_CHECK_EQ(path.containerSize(), 4);
	PR_CHECK_TRUE(path.isEmpty());

	path.addToken(LightPathToken::Camera());
	path.addToken(LightPathToken(ST_REFLECTION, SE_DIFFUSE));
	PR_CHECK_EQ(path.currentSize(), 2);
	PR_CHECK_EQ(path.containerSize(), 4);
	PR_CHECK_FALSE(path.isEmpty());

	path.reset();
	PR_CHECK_EQ(path.currentSize(), 0);
	PR_CHECK_EQ(path.containerSize(), 4);
	PR_CHECK_TRUE(path.isEmpty());
}
/*PR_TEST("[C[^S]+S?B]")
{
	LightPathExpression expr("C[^S]+S?B");
	PR_CHECK_TRUE(expr.isValid());
}
PR_TEST("[C[^<.S\"Plane\">]+S?B]")
{
	LightPathExpression expr("C[^<.S\"Plane\">]+S?B");
	PR_CHECK_TRUE(expr.isValid());
}*/

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(LPE);
PRT_END_MAIN