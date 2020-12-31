#include "path/LightPath.h"
#include "path/LightPathExpression.h"
#include "path/LightPathView.h"

#include "Test.h"

using namespace PR;

PR_BEGIN_TESTCASE(LPE)
PR_TEST("[CD*L]")
{
	LightPathExpression expr("CD*L");
	PR_CHECK_TRUE(expr.isValid());

	LightPath path; // CDE
	path.addToken(LightPathToken::Camera());
	path.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path.addToken(LightPathToken(ScatteringType::Emissive, ScatteringEvent::Diffuse));
	PR_CHECK_TRUE(expr.match(path));

	LightPath path2; // CE
	path2.addToken(LightPathToken::Camera());
	path2.addToken(LightPathToken(ScatteringType::Emissive, ScatteringEvent::Diffuse));
	PR_CHECK_TRUE(expr.match(path2));

	LightPath path3; // CSE
	path3.addToken(LightPathToken::Camera());
	path3.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Specular));
	path3.addToken(LightPathToken(ScatteringType::Emissive, ScatteringEvent::Diffuse));
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
	path.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path.addToken(LightPathToken(ScatteringType::Refraction, ScatteringEvent::Specular));
	path.addToken(LightPathToken(ScatteringType::Emissive, ScatteringEvent::Diffuse));
	PR_CHECK_TRUE(expr.match(path));

	LightPath path2; // CDSDSDE
	path2.addToken(LightPathToken::Camera());
	path2.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path2.addToken(LightPathToken(ScatteringType::Refraction, ScatteringEvent::Specular));
	path2.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path2.addToken(LightPathToken(ScatteringType::Refraction, ScatteringEvent::Specular));
	path2.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path2.addToken(LightPathToken(ScatteringType::Emissive, ScatteringEvent::Diffuse));
	PR_CHECK_TRUE(expr.match(path2));

	LightPath path3; // CSE
	path3.addToken(LightPathToken::Camera());
	path3.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Specular));
	path3.addToken(LightPathToken(ScatteringType::Emissive, ScatteringEvent::Diffuse));
	PR_CHECK_FALSE(expr.match(path3));

	LightPath path4; // CSDE
	path4.addToken(LightPathToken::Camera());
	path4.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Specular));
	path4.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path4.addToken(LightPathToken(ScatteringType::Emissive, ScatteringEvent::Diffuse));
	PR_CHECK_FALSE(expr.match(path4));
}
PR_TEST("[C[DS]+D?B]")
{
	LightPathExpression expr("C[DS]+D?B");
	PR_CHECK_TRUE(expr.isValid());

	LightPath path; // CDSB
	path.addToken(LightPathToken::Camera());
	path.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path.addToken(LightPathToken(ScatteringType::Refraction, ScatteringEvent::Specular));
	path.addToken(LightPathToken::Background());
	PR_CHECK_TRUE(expr.match(path));

	LightPath path2; // CDSDDB
	path2.addToken(LightPathToken::Camera());
	path2.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path2.addToken(LightPathToken(ScatteringType::Refraction, ScatteringEvent::Specular));
	path2.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path2.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path2.addToken(LightPathToken::Background());
	PR_CHECK_TRUE(expr.match(path2));

	LightPath path3; // CEB
	path3.addToken(LightPathToken::Camera());
	path3.addToken(LightPathToken(ScatteringType::Emissive, ScatteringEvent::Specular));
	path3.addToken(LightPathToken::Background());
	PR_CHECK_FALSE(expr.match(path3));
}
PR_TEST("[C(DS+)+.*L]")
{
	LightPathExpression expr("C(DS+)+.*L");
	PR_CHECK_TRUE(expr.isValid());

	//std::cout << expr.dumpTable() << std::endl;
	//std::cout << LightPathExpression::generateDotString("C(DS+)+.*L") << std::endl;

	LightPath path; // CDSB
	path.addToken(LightPathToken::Camera());
	path.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path.addToken(LightPathToken(ScatteringType::Refraction, ScatteringEvent::Specular));
	path.addToken(LightPathToken::Background());
	PR_CHECK_TRUE(expr.match(path));

	LightPath path2; // CDSDDB
	path2.addToken(LightPathToken::Camera());
	path2.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path2.addToken(LightPathToken(ScatteringType::Refraction, ScatteringEvent::Specular));
	path2.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path2.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path2.addToken(LightPathToken::Background());
	PR_CHECK_TRUE(expr.match(path2)); //f

	LightPath path3; // CDSSDSDDB
	path3.addToken(LightPathToken::Camera());
	path3.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path3.addToken(LightPathToken(ScatteringType::Refraction, ScatteringEvent::Specular));
	path3.addToken(LightPathToken(ScatteringType::Refraction, ScatteringEvent::Specular));
	path3.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path3.addToken(LightPathToken(ScatteringType::Refraction, ScatteringEvent::Specular));
	path3.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path3.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path3.addToken(LightPathToken::Background());
	PR_CHECK_TRUE(expr.match(path3));

	LightPath path4; // CEB
	path4.addToken(LightPathToken::Camera());
	path4.addToken(LightPathToken(ScatteringType::Emissive, ScatteringEvent::Specular));
	path4.addToken(LightPathToken::Background());
	PR_CHECK_FALSE(expr.match(path4));
}
PR_TEST("Add/Pop")
{
	LightPath path; // CDSB
	PR_CHECK_TRUE(path.isEmpty());

	path.addToken(LightPathToken::Camera());
	path.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path.addToken(LightPathToken(ScatteringType::Refraction, ScatteringEvent::Specular));
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
	path.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	PR_CHECK_EQ(path.currentSize(), 2);
	PR_CHECK_EQ(path.containerSize(), 4);
	PR_CHECK_FALSE(path.isEmpty());

	path.reset();
	PR_CHECK_EQ(path.currentSize(), 0);
	PR_CHECK_EQ(path.containerSize(), 4);
	PR_CHECK_TRUE(path.isEmpty());
}
PR_TEST("Pop Until")
{
	LightPath path; // CDSB
	PR_CHECK_TRUE(path.isEmpty());

	path.addToken(LightPathToken::Camera());
	path.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path.addToken(LightPathToken(ScatteringType::Refraction, ScatteringEvent::Specular));
	path.addToken(LightPathToken::Background());

	PR_CHECK_EQ(path.currentSize(), 4);
	PR_CHECK_EQ(path.containerSize(), 4);
	PR_CHECK_FALSE(path.isEmpty());

	path.popTokenUntil(1);
	PR_CHECK_EQ(path.currentSize(), 1);
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
PR_TEST("Packed")
{
	LightPath path; // CDSB
	path.addToken(LightPathToken::Camera());
	path.addToken(LightPathToken(ScatteringType::Reflection, ScatteringEvent::Diffuse));
	path.addToken(LightPathToken(ScatteringType::Refraction, ScatteringEvent::Specular));
	path.addToken(LightPathToken::Background());

	size_t size = path.currentSize();

	uint32* buffer = new uint32[size + 1];
	path.toPacked((uint8*)buffer, (size + 1) * sizeof(uint32));

	PR_CHECK_EQ(path.packedSizeRequirement(), (size + 1) * sizeof(uint32));

	LightPathView view = LightPathView(buffer);
	PR_CHECK_EQ(view.currentSize(), size);
	for (size_t i = 0; i < size; ++i) {
		PR_CHECK_EQ(view.token(i).Type, path.token(i).Type);
		PR_CHECK_EQ(view.token(i).Event, path.token(i).Event);
		PR_CHECK_EQ(view.token(i).LabelIndex, path.token(i).LabelIndex);
	}

	delete[] buffer;
}

PR_END_TESTCASE()

// MAIN
PRT_BEGIN_MAIN
PRT_TESTCASE(LPE);
PRT_END_MAIN