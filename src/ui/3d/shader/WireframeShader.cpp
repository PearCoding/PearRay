#include "WireframeShader.h"

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

static const char* geometryShaderSource = PRUI_SHADER_HEADER
	"layout (triangles) in;\n"
	"layout (line_strip, max_vertices=3) out;\n"
	""
	"void main(void) {\n"
	"    for (int i = 0; i < gl_in.length(); i++) {\n"
	"        gl_Position = gl_in[i].gl_Position; //Pass through\n"
	"        EmitVertex();\n"
	"    }\n"
	"    EndPrimitive();\n"
	"}";

static const char* fragmentShaderSource = PRUI_SHADER_HEADER
	"uniform lowp vec4 flatColor;\n"
	""
	"out vec4 fragcolor;\n"
	""
	"void main() {\n"
	"  fragcolor = flatColor;\n"
	"}\n";

WireframeShader::WireframeShader(const Vector4f& c)
	: Shader(vertexShaderSource, geometryShaderSource, fragmentShaderSource)
	, mColorUniform(0)
	, mColor(c)
{
}

void WireframeShader::onInit()
{
	mColorUniform = program()->uniformLocation("flatColor");
}

void WireframeShader::onBind(const ShadingContext&)
{
	program()->setUniformValue(mColorUniform, mColor);
}

void WireframeShader::updateFlags()
{
	enableTransparency(mColor(3) < 1);
}
} // namespace UI
} // namespace PR