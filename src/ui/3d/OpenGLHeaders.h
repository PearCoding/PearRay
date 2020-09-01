#pragma once

#include "Logger.h"
#include "glad.h"

// Workaround for qt
#undef APIENTRY
#define __gl_h_
#define __glext_h_

#include <iostream>

#define _GL_CHECK(_call)                                                                                               \
	do {                                                                                                               \
		_call;                                                                                                         \
		GLenum gl_err = glGetError();                                                                                  \
		if (0 != gl_err)                                                                                               \
			PR_LOG(PR::L_ERROR) << #_call ">> GL error " << gl_err << ": " << PR::UI::glEnumName(gl_err) << std::endl; \
	} while (false)

// Check return value of glGetError assuming the function was already called
#define _GL_CHECK_PREV(_call)                                                                                          \
	do {                                                                                                               \
		GLenum gl_err = glGetError();                                                                                  \
		if (0 != gl_err)                                                                                               \
			PR_LOG(PR::L_ERROR) << #_call ">> GL error " << gl_err << ": " << PR::UI::glEnumName(gl_err) << std::endl; \
	} while (false)

#if _DEBUG
#define GL_CHECK(_call) _GL_CHECK(_call)
#define GL_CHECK_PREV(_call) _GL_CHECK_PREV(_call)
#else
#define GL_CHECK(_call) _call
#define GL_CHECK_PREV(_call) \
	do {                     \
	} while (false)
#endif

namespace PR {
namespace UI {
inline const char* glEnumName(GLenum code)
{
	switch (code) {
	case GL_NO_ERROR:
		return "No Error";
	case GL_INVALID_OPERATION:
		return "Invalid Operation";
	case GL_INVALID_ENUM:
		return "Invalid Enum";
	case GL_INVALID_VALUE:
		return "Invalid Value";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "Invalid Framebuffer Operation";
	case GL_OUT_OF_MEMORY:
		return "Out of Memory";
	default:
		return "Unknown Error";
	}
}

inline bool init_opengl_functions(GLADloadproc loader)
{
	return gladLoadGLLoader(loader);
}
} // namespace UI
} // namespace PR