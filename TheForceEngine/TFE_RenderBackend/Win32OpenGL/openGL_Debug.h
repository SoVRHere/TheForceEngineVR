#pragma once

#include "gl.h"
#include <cstdint>

namespace OpenGL_Debug
{
	void Initialize();

	void AssertGL(const char* file, int line);
	const char* GetConstStr(GLenum constant);
	bool CheckRenderTargetStatus(GLuint target);

	enum class GLObjectType
	{
		Shader = 0,
		Program,
		Buffer,
		Texture,
		Framebuffer,
		Count
	};

	void SetObjectName(GLObjectType identifier, GLuint id, const char* name);
	void PushGroup(const char* label);
	void PopGroup();
}

#ifdef _DEBUG
#define TFE_ASSERT_GL OpenGL_Debug::AssertGL(__FILE__, __LINE__)
#else
#define TFE_ASSERT_GL
#endif
