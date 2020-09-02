#include "BackgroundShader.h"

namespace PR {
namespace UI {
static const char* vertexShaderSource = PRUI_SHADER_HEADER
	"out mediump vec2 uv;\n"
	""
	"void main() {\n"
	"  uint idx = uint(gl_VertexID);\n"
	"  gl_Position = vec4(idx & 1U, idx >> 1U, 0.0, 0.5) * 4.0 - 1.0;\n"
	"  uv = vec2(gl_Position.xy * 0.5 + 0.5);\n"
	"}\n";

static const char* fragmentShaderSource = PRUI_SHADER_HEADER
	"precision mediump float;\n"
	"in mediump vec2 uv;\n"
	""
	"uniform vec4 color_start;\n"
	"uniform vec4 color_end;\n"
	""
	"out vec4 fragcolor;\n"
	""
	"void main() {\n"
	"  fragcolor = mix(color_end, color_start, uv.y);\n"
	"}\n";

BackgroundShader::BackgroundShader()
	: Shader(vertexShaderSource, fragmentShaderSource)
	, mColorStart(0.6f, 0.7f, 0.8f, 1)
	, mColorEnd(0.05f, 0.05f, 0.05f, 1)
{
}

void BackgroundShader::onInit()
{
	mColorStartUniform = program()->uniformLocation("color_start");
	mColorEndUniform   = program()->uniformLocation("color_end");
}

void BackgroundShader::onBind(const ShadingContext&)
{
	program()->setUniformValue(mColorStartUniform, mColorStart);
	program()->setUniformValue(mColorEndUniform, mColorEnd);
}
} // namespace UI
} // namespace PR