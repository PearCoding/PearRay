#include "Shader.h"
#include "Logger.h"

namespace PR {
namespace UI {
Shader::Shader()
	: mInitialized(false)
	, mFlags(0)
	, mPositionAttrib(-1)
	, mNormalAttrib(-1)
	, mColorAttrib(-1)
	, mWeightAttrib(-1)
	, mMatrixUniform(-1)
{
}

Shader::~Shader() {}

Shader::Shader(const std::string& vertexShader, const std::string& fragmentShader)
	: Shader()
{
	mVertexShader	= vertexShader;
	mFragmentShader = fragmentShader;
}

void Shader::create()
{
	if (!mInitialized)
		initialize();
}

void Shader::bind(const ShadingContext& sc)
{
	if (!mInitialized)
		initialize();

	if (!isValid())
		return;

	mProgram->bind();
	if (mMatrixUniform >= 0)
		mProgram->setUniformValue(mMatrixUniform, sc.FullMatrix);

	onBind(sc);
}

void Shader::onInit()
{
	// Nothing
}

void Shader::onBind(const ShadingContext& sc)
{
	(void)sc;
	// Nothing
}

void Shader::initialize()
{
	mInitialized = true;
	mFlags		 = 0;

	mProgram = std::make_unique<ShaderProgram>();

	mProgram->addVertexShader(mVertexShader);
	mProgram->addFragmentShader(mFragmentShader);
	if (!mProgram->link()) {
		PR_LOG(L_ERROR) << "GLSL: " << mProgram->log() << std::endl;
		mProgram.reset();
		return;
	}

	mPositionAttrib = mProgram->attributeLocation("positionAttrib");
	if (mPositionAttrib >= 0)
		mFlags |= SF_HasPositionAttrib;

	mNormalAttrib = mProgram->attributeLocation("normalAttrib");
	if (mNormalAttrib >= 0)
		mFlags |= SF_HasNormalAttrib;

	mColorAttrib = mProgram->attributeLocation("colorAttrib");
	if (mColorAttrib >= 0)
		mFlags |= SF_HasColorAttrib;

	mWeightAttrib = mProgram->attributeLocation("weightAttrib");
	if (mWeightAttrib >= 0)
		mFlags |= SF_HasWeightAttrib;

	mMatrixUniform = mProgram->uniformLocation("matrix");

	onInit();
}
} // namespace UI
} // namespace PR