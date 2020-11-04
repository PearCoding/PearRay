#include "GoochShader.h"

namespace PR {
namespace UI {
static const char* vertexShaderSource = PRUI_SHADER_HEADER
	"layout(location = 0) in highp vec4 positionAttrib;\n"
	"layout(location = 1) in highp vec4 normalAttrib;\n"
	""
	"out highp vec3 normal;\n"
	""
	"uniform highp mat3 normmatrix;\n"
	"uniform highp mat4 matrix;\n"
	""
	"void main() {\n"
	"  normal = normalize(normmatrix * normalAttrib.xyz);\n"
	"  gl_Position = matrix * positionAttrib;\n"
	"}\n";

static const char* fragmentShaderSource = PRUI_SHADER_HEADER
	"precision mediump float;\n"
	"in highp vec3 normal;\n"
	""
	"uniform lowp vec4 colorWarm;\n"
	"uniform lowp vec4 colorCold;\n"
	""
	"out vec4 fragcolor;\n"
	""
	"void main() {\n"
	"  float t = (1.0 + normal.z) / 2.0;\n" // In view space y is the direction towards camera, so we light it from the top (z-axis)
	"  fragcolor = mix(colorCold, colorWarm, t);\n"
	"}\n";

GoochShader::GoochShader()
	: Shader(vertexShaderSource, fragmentShaderSource)
	, mColorWarmUniform(0)
	, mColorColdUniform(0)
	, mNormalMatUniform(0)
{
}

void GoochShader::onInit()
{
	mColorWarmUniform = program()->uniformLocation("colorWarm");
	mColorColdUniform = program()->uniformLocation("colorCold");
	mNormalMatUniform = program()->uniformLocation("normmatrix");
}

void GoochShader::onBind(const ShadingContext& sc)
{
	program()->setUniformValue(mColorWarmUniform, mColorWarm);
	program()->setUniformValue(mColorColdUniform, mColorCold);
	program()->setUniformValue(mNormalMatUniform, sc.computeNormalMatrix());
}

void GoochShader::updateFlags()
{
	enableTransparency(mColorWarm(3) < 1 || mColorCold(3) < 1);
}
} // namespace UI
} // namespace PR