#include "ColorShader.h"

namespace PR {
namespace UI {
static const char* vertexShaderSource = PRUI_SHADER_HEADER
	"layout(location = 0) in highp vec4 positionAttrib;\n"
	""
	"uniform highp mat4 matrix;\n"
	""
	"void main() {\n"
	"  gl_Position = matrix * positionAttrib;\n"
	"}\n";

static const char* fragmentShaderSource = PRUI_SHADER_HEADER
	"uniform lowp vec4 color;\n"
	""
	"out vec4 fragcolor;\n"
	""
	"void main() {\n"
	"  fragcolor = color;\n"
	"}\n";

ColorShader::ColorShader(const Vector4f& c)
	: Shader(vertexShaderSource, fragmentShaderSource)
	, mColor(c)
{
}

void ColorShader::onInit()
{
	mColorUniform = program()->uniformLocation("color");
}

void ColorShader::onBind(const ShadingContext&)
{
	program()->setUniformValue(mColorUniform, mColor);
}

void ColorShader::updateFlags()
{
	enableTransparency(mColor(3) < 1);
}
} // namespace UI
} // namespace PR