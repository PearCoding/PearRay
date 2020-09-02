#include "ColorShader.h"

namespace PR {
namespace UI {
static const char* vertexShaderSource = PRUI_SHADER_HEADER
	"layout(location = 0) in highp vec4 positionAttrib;\n"
	"layout(location = 2) in mediump vec4 colorAttrib;\n"
	""
	"out vec4 vertexColor;\n"
	""
	"uniform highp mat4 matrix;\n"
	""
	"void main() {\n"
	"  gl_Position = matrix * positionAttrib;\n"
	"  vertexColor = colorAttrib;\n"
	"}\n";

static const char* fragmentShaderSource = PRUI_SHADER_HEADER
	"uniform lowp vec4 flatColor;\n"
	"uniform float blendFactor;\n"
	""
	"in vec4 vertexColor;\n"
	""
	"out vec4 fragcolor;\n"
	""
	"void main() {\n"
	"  fragcolor = mix(vertexColor, flatColor, blendFactor);\n"
	"}\n";

ColorShader::ColorShader()
	: Shader(vertexShaderSource, fragmentShaderSource)
	, mColor(Vector4f::Zero())
	, mBlendFactor(0)
{
}

ColorShader::ColorShader(const Vector4f& c)
	: Shader(vertexShaderSource, fragmentShaderSource)
	, mColor(c)
	, mBlendFactor(1)
{
}

void ColorShader::onInit()
{
	mColorUniform = program()->uniformLocation("flatColor");
	mBlendUniform = program()->uniformLocation("blendFactor");
}

void ColorShader::onBind(const ShadingContext&)
{
	program()->setUniformValue(mColorUniform, mColor);
	program()->setUniformValue(mBlendUniform, mBlendFactor);
}

void ColorShader::updateFlags()
{
	enableTransparency(mColor(3) < 1);
}
} // namespace UI
} // namespace PR