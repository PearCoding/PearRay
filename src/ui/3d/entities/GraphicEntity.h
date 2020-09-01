#pragma once

#include "3d/shader/Shader.h"
#include "TransformableEntity.h"

namespace PR {
namespace UI {
enum class GraphicAttributes : int {
	Position = 0,
	Normal	 = 1,
	Color	 = 2,
	Weight	 = 3
};

class PR_LIB_UI GraphicEntity : public TransformableEntity {
public:
	GraphicEntity();
	virtual ~GraphicEntity();

	virtual void render(const ShadingContext& sc);
	virtual void create();

	inline void setShader(const std::shared_ptr<Shader>& shader) { mShader = shader; }
	inline std::shared_ptr<Shader> shader() const { return mShader; }

	inline bool isTwoSided() const { return mTwoSided; }
	inline void setTwoSided(bool b) { mTwoSided = b; }

	inline const std::vector<float>& vertices() const { return mVertices; }
	inline const std::vector<float>& normals() const { return mNormals; }
	inline const std::vector<float>& colors() const { return mColors; }
	inline const std::vector<float>& weights() const { return mWeights; }
	inline const std::vector<unsigned int>& indices() const { return mIndices; }

protected:
	virtual void onBeforeRender();
	virtual void onAfterRender();

	inline std::vector<float>& rawVertices() { return mVertices; }
	inline std::vector<float>& rawNormals() { return mNormals; }
	inline std::vector<float>& rawColors() { return mColors; }
	inline std::vector<float>& rawWeights() { return mWeights; }
	inline std::vector<unsigned int>& rawIndices() { return mIndices; }

	inline void setVertices(const std::vector<float>& v)
	{
		mVertices		= v;
		mRequireRebuild = true;
	}

	inline void setNormals(const std::vector<float>& v)
	{
		mNormals		= v;
		mRequireRebuild = true;
	}

	inline void setColors(const std::vector<float>& v)
	{
		mColors			= v;
		mRequireRebuild = true;
	}

	inline void setWeights(const std::vector<float>& v)
	{
		mWeights		= v;
		mRequireRebuild = true;
	}

	inline void setIndices(const std::vector<unsigned int>& v) { mIndices = v; }
	inline void setDrawMode(int mode) { mDrawMode = mode; }

	inline void useProxyVertexSize(size_t i) { mProxyVertexSize = i; }

	inline void requestRebuild() { mRequireRebuild = true; }

private:
	std::shared_ptr<Shader> mShader;

	std::vector<float> mVertices; // 3D float attribute
	std::vector<float> mNormals;  // 3D float attribute
	std::vector<float> mColors;	  // 3D float attribute
	std::vector<float> mWeights;  // 1D float attribute
	std::vector<unsigned int> mIndices;
	int mDrawMode;
	bool mTwoSided;

	size_t mProxyVertexSize;

	bool mRequireRebuild;

	unsigned int mVAO;
	unsigned int mVBO_Vertices;
	unsigned int mVBO_Normals;
	unsigned int mVBO_Colors;
	unsigned int mVBO_Weights;
};
} // namespace UI
}