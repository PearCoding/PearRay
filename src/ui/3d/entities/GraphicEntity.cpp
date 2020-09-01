#include "3d/OpenGLHeaders.h"

#include "GraphicEntity.h"

namespace PR {
namespace UI {
GraphicEntity::GraphicEntity()
	: TransformableEntity()
	, mDrawMode(GL_TRIANGLES)
	, mTwoSided(false)
	, mProxyVertexSize(0)
	, mRequireRebuild(true)
	, mVAO(0)
	, mVBO_Vertices(0)
	, mVBO_Normals(0)
	, mVBO_Colors(0)
	, mVBO_Weights(0)
{
}

GraphicEntity::~GraphicEntity()
{
	if (mVBO_Vertices != 0)
		GL_CHECK(glDeleteBuffers(1, &mVBO_Vertices));
	if (mVBO_Normals != 0)
		GL_CHECK(glDeleteBuffers(1, &mVBO_Normals));
	if (mVBO_Colors != 0)
		GL_CHECK(glDeleteBuffers(1, &mVBO_Colors));
	if (mVBO_Weights != 0)
		GL_CHECK(glDeleteBuffers(1, &mVBO_Weights));
	if (mVAO != 0)
		GL_CHECK(glDeleteVertexArrays(1, &mVAO));
}
void GraphicEntity::create()
{
	if (mVAO == 0)
		GL_CHECK(glGenVertexArrays(1, &mVAO));
	GL_CHECK(glBindVertexArray(mVAO));

	// Vertices
	if (!mVertices.empty()) {
		if (mVBO_Vertices == 0)
			GL_CHECK(glGenBuffers(1, &mVBO_Vertices));

		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mVBO_Vertices));
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(float), mVertices.data(), GL_STATIC_DRAW));
		GL_CHECK(glVertexAttribPointer((GLuint)GraphicAttributes::Position, 3, GL_FLOAT, GL_FALSE, 0, 0));
		GL_CHECK(glEnableVertexAttribArray((GLuint)GraphicAttributes::Position));
	}

	// Normals
	if (!mNormals.empty()) {
		if (mVBO_Normals == 0)
			GL_CHECK(glGenBuffers(1, &mVBO_Normals));

		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mVBO_Normals));
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, mNormals.size() * sizeof(float), mNormals.data(), GL_STATIC_DRAW));
		GL_CHECK(glVertexAttribPointer((GLuint)GraphicAttributes::Normal, 3, GL_FLOAT, GL_FALSE, 0, 0));
		GL_CHECK(glEnableVertexAttribArray((GLuint)GraphicAttributes::Normal));
	}

	// Colors
	if (!mColors.empty()) {
		if (mVBO_Colors == 0)
			GL_CHECK(glGenBuffers(1, &mVBO_Colors));

		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mVBO_Colors));
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, mColors.size() * sizeof(float), mColors.data(), GL_STATIC_DRAW));
		GL_CHECK(glVertexAttribPointer((GLuint)GraphicAttributes::Color, 3, GL_FLOAT, GL_FALSE, 0, 0));
		GL_CHECK(glEnableVertexAttribArray((GLuint)GraphicAttributes::Color));
	}

	// Weights
	if (!mWeights.empty()) {
		if (mVBO_Weights == 0)
			GL_CHECK(glGenBuffers(1, &mVBO_Weights));

		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, mVBO_Weights));
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, mWeights.size() * sizeof(float), mWeights.data(), GL_STATIC_DRAW));
		GL_CHECK(glVertexAttribPointer((GLuint)GraphicAttributes::Weight, 1, GL_FLOAT, GL_FALSE, 0, 0));
		GL_CHECK(glEnableVertexAttribArray((GLuint)GraphicAttributes::Weight));
	}

	mRequireRebuild = false;
}

void GraphicEntity::render(const ShadingContext& sc)
{
	if (mVertices.empty() && mProxyVertexSize == 0)
		return;

	if (!mShader)
		return;

	if (mVAO == 0 || mRequireRebuild)
		create();
	else
		GL_CHECK(glBindVertexArray(mVAO));

	GLboolean cullFace;
	GL_CHECK(glGetBooleanv(GL_CULL_FACE, &cullFace));
	if (mTwoSided && cullFace == GL_TRUE)
		GL_CHECK(glDisable(GL_CULL_FACE));
	else if (!mTwoSided && cullFace == GL_FALSE)
		GL_CHECK(glEnable(GL_CULL_FACE));

	mShader->bind(sc.chain(transform()));

	onBeforeRender();

	if (mIndices.empty()) {
		if (mVertices.empty())
			GL_CHECK(glDrawArrays(mDrawMode, 0, mProxyVertexSize));
		else
			GL_CHECK(glDrawArrays(mDrawMode, 0, mVertices.size() / 3));
	} else
		GL_CHECK(glDrawElements(mDrawMode, mIndices.size(), GL_UNSIGNED_INT, mIndices.data()));

	// Restore previous cull state
	if (mTwoSided && cullFace == GL_TRUE)
		GL_CHECK(glEnable(GL_CULL_FACE));
	else if (!mTwoSided && cullFace == GL_FALSE)
		GL_CHECK(glDisable(GL_CULL_FACE));

	onAfterRender();
}

void GraphicEntity::onBeforeRender()
{
	// Nothing
}

void GraphicEntity::onAfterRender()
{
	// Nothing
}
} // namespace UI
} // namespace PR