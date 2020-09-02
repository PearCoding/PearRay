#pragma once
#include "PR_Config.h"

#define PRUI_SHADER_HEADER "#version 330\n"

namespace PR {
namespace UI {
class PR_LIB_UI ShaderProgram {
public:
	ShaderProgram();
	ShaderProgram(const std::string& vertexShader, const std::string& fragmentShader);
	ShaderProgram(const std::string& vertexShader, const std::string& geometryShader, const std::string& fragmentShader);
	virtual ~ShaderProgram();

	inline void addVertexShader(const std::string& str) { mVertexShader = str; }
	inline void addGeometryShader(const std::string& str) { mGeometryShader = str; }
	inline void addFragmentShader(const std::string& str) { mFragmentShader = str; }
	bool create();
	bool link();

	inline bool isCreated() const { return mProgram > 0; }
	inline bool isLinked() const { return mLinked; }

	bool bind();

	inline const std::string& log() const { return mLastLog; }

	int attributeLocation(const std::string& name) const;
	int uniformLocation(const std::string& name) const;

	void setAttributeValue(int location, float v);
	void setAttributeValue(int location, const Vector2f& v);
	void setAttributeValue(int location, const Vector3f& v);
	void setAttributeValue(int location, const Vector4f& v);

	void setUniformValue(int location, unsigned int v);
	void setUniformValue(int location, int v);
	void setUniformValue(int location, float v);
	void setUniformValue(int location, const Vector2f& v);
	void setUniformValue(int location, const Vector3f& v);
	void setUniformValue(int location, const Vector4f& v);
	void setUniformValue(int location, const Matrix3f& v);
	void setUniformValue(int location, const Matrix4f& v);

	int internalHandle() const { return mProgram; }

private:
	std::string mVertexShader;
	std::string mGeometryShader;
	std::string mFragmentShader;

	bool mLinked;
	std::string mLastLog;

	int mProgram;
};
} // namespace UI
} // namespace PR