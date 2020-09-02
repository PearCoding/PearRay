#include "ShaderProgram.h"
#include "3d/OpenGLHeaders.h"

namespace PR {
namespace UI {
ShaderProgram::ShaderProgram()
	: mLinked(false)
	, mProgram(-1)
{
}

ShaderProgram::ShaderProgram(const std::string& vertexShader, const std::string& fragmentShader)
	: ShaderProgram()
{
	mVertexShader	= vertexShader;
	mFragmentShader = fragmentShader;
}

ShaderProgram::ShaderProgram(const std::string& vertexShader, const std::string& geometryShader, const std::string& fragmentShader)
	: ShaderProgram()
{
	mVertexShader	= vertexShader;
	mGeometryShader = geometryShader;
	mFragmentShader = fragmentShader;
}

ShaderProgram::~ShaderProgram()
{
	if (isCreated())
		GL_CHECK(glDeleteProgram(mProgram));
}

bool ShaderProgram::create()
{
	if (isCreated())
		return true;
	mProgram = glCreateProgram();
	GL_CHECK_PREV(glCreateProgram);
	return isCreated();
}

static std::string retriveShaderLog(int handle)
{
	int length	= 0;
	int written = 0;
	char* infoLog;

	GL_CHECK(glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &length));

	if (length > 0) {
		infoLog = new char[length];
		GL_CHECK(glGetShaderInfoLog(handle, length, &written, infoLog));
		std::string log = infoLog;
		delete[] infoLog;
		return log;
	}

	return "";
}

static std::string retriveProgramLog(int handle)
{
	int length	= 0;
	int written = 0;
	char* infoLog;

	GL_CHECK(glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &length));

	if (length > 0) {
		infoLog = new char[length];
		GL_CHECK(glGetProgramInfoLog(handle, length, &written, infoLog));
		std::string log = infoLog;
		delete[] infoLog;
		return log;
	}

	return "";
}

bool ShaderProgram::link()
{
	if (!isCreated()) {
		if (!create())
			return false;
	}

	// Vertex
	int v_handle = glCreateShader(GL_VERTEX_SHADER);
	GL_CHECK_PREV(glCreateShader);

	const char* v_src = mVertexShader.c_str();
	int v_len		  = mVertexShader.size();
	if (v_len <= 0) {
		GL_CHECK(glDeleteShader(v_handle));
		mLastLog = "No vertex shader given";
		return false;
	}

	GL_CHECK(glShaderSource(v_handle, 1, &v_src, &v_len));
	GL_CHECK(glCompileShader(v_handle));
	int status;
	GL_CHECK(glGetShaderiv(v_handle, GL_COMPILE_STATUS, &status));
	if (status != GL_TRUE) {
		mLastLog = retriveShaderLog(v_handle);
		GL_CHECK(glDeleteShader(v_handle));
		return false;
	}

	int g_handle = -1;
	if (!mGeometryShader.empty()) {
		g_handle = glCreateShader(GL_GEOMETRY_SHADER);
		GL_CHECK_PREV(glCreateShader);

		const char* g_src = mGeometryShader.c_str();
		int g_len		  = mGeometryShader.size();
		PR_ASSERT(g_len > 0, "empty geometry shader given but not detected");

		GL_CHECK(glShaderSource(g_handle, 1, &g_src, &g_len));
		GL_CHECK(glCompileShader(g_handle));
		GL_CHECK(glGetShaderiv(g_handle, GL_COMPILE_STATUS, &status));
		if (status != GL_TRUE) {
			mLastLog = retriveShaderLog(g_handle);
			GL_CHECK(glDeleteShader(v_handle));
			GL_CHECK(glDeleteShader(g_handle));
			return false;
		}
	}

	// Fragment
	int f_handle = glCreateShader(GL_FRAGMENT_SHADER);
	GL_CHECK_PREV(glCreateShader);

	const char* f_src = mFragmentShader.c_str();
	int f_len		  = mFragmentShader.size();
	if (f_len <= 0) {
		GL_CHECK(glDeleteShader(v_handle));
		if (g_handle >= 0)
			GL_CHECK(glDeleteShader(g_handle));
		GL_CHECK(glDeleteShader(f_handle));
		mLastLog = "No fragment shader given";
		return false;
	}

	GL_CHECK(glShaderSource(f_handle, 1, &f_src, &f_len));
	GL_CHECK(glCompileShader(f_handle));
	GL_CHECK(glGetShaderiv(f_handle, GL_COMPILE_STATUS, &status));
	if (status != GL_TRUE) {
		mLastLog = retriveShaderLog(f_handle);
		GL_CHECK(glDeleteShader(v_handle));
		if (g_handle >= 0)
			GL_CHECK(glDeleteShader(g_handle));
		GL_CHECK(glDeleteShader(f_handle));
		return false;
	}

	GL_CHECK(glAttachShader(mProgram, v_handle));
	if (g_handle >= 0)
		GL_CHECK(glAttachShader(mProgram, g_handle));
	GL_CHECK(glAttachShader(mProgram, f_handle));
	GL_CHECK(glLinkProgram(mProgram));

	GL_CHECK(glDeleteShader(v_handle));
	if (g_handle >= 0)
		GL_CHECK(glDeleteShader(g_handle));
	GL_CHECK(glDeleteShader(f_handle));

	GL_CHECK(glGetProgramiv(mProgram, GL_LINK_STATUS, &status));
	if (status != GL_TRUE) {
		mLastLog = retriveProgramLog(mProgram);
		return false;
	}

	mLinked = true;
	return true;
}

bool ShaderProgram::bind()
{
	if (!isLinked()) {
		if (!link())
			return false;
	}

	GL_CHECK(glUseProgram(mProgram));
	return true;
}

int ShaderProgram::attributeLocation(const std::string& name) const
{
	if (!isLinked())
		return -1;
	int loc = glGetAttribLocation(mProgram, name.c_str());
	GL_CHECK_PREV(glGetAttribLocation);
	return loc;
}

int ShaderProgram::uniformLocation(const std::string& name) const
{
	if (!isLinked())
		return -1;
	int loc = glGetUniformLocation(mProgram, name.c_str());
	GL_CHECK_PREV(glGetUniformLocation);
	return loc;
}

void ShaderProgram::setAttributeValue(int location, float v) { GL_CHECK(glVertexAttrib1f(location, v)); }
void ShaderProgram::setAttributeValue(int location, const Vector2f& v) { GL_CHECK(glVertexAttrib2f(location, v.x(), v.y())); }
void ShaderProgram::setAttributeValue(int location, const Vector3f& v) { GL_CHECK(glVertexAttrib3f(location, v.x(), v.y(), v.z())); }
void ShaderProgram::setAttributeValue(int location, const Vector4f& v) { GL_CHECK(glVertexAttrib4f(location, v.x(), v.y(), v.z(), v.w())); }

void ShaderProgram::setUniformValue(int location, unsigned int v) { GL_CHECK(glUniform1ui(location, v)); }
void ShaderProgram::setUniformValue(int location, int v) { GL_CHECK(glUniform1i(location, v)); }
void ShaderProgram::setUniformValue(int location, float v) { GL_CHECK(glUniform1f(location, v)); }
void ShaderProgram::setUniformValue(int location, const Vector2f& v) { GL_CHECK(glUniform2f(location, v.x(), v.y())); }
void ShaderProgram::setUniformValue(int location, const Vector3f& v) { GL_CHECK(glUniform3f(location, v.x(), v.y(), v.z())); }
void ShaderProgram::setUniformValue(int location, const Vector4f& v) { GL_CHECK(glUniform4f(location, v.x(), v.y(), v.z(), v.w())); }
void ShaderProgram::setUniformValue(int location, const Matrix3f& v) { GL_CHECK(glUniformMatrix3fv(location, 1, GL_FALSE, v.data())); }
void ShaderProgram::setUniformValue(int location, const Matrix4f& v) { GL_CHECK(glUniformMatrix4fv(location, 1, GL_FALSE, v.data())); }
} // namespace UI
} // namespace PR