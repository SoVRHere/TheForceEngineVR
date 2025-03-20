#include "glslParser.h"
#include <TFE_RenderBackend/shader.h>
#include <TFE_RenderBackend/vertexBuffer.h>
#include <TFE_RenderBackend/Win32OpenGL/openGL_Debug.h>
#include <TFE_RenderBackend/Win32OpenGL/openGL_Caps.h>
#include <TFE_System/system.h>
#include <TFE_FileSystem/filestream.h>
#include <TFE_FileSystem/paths.h>
#include <TFE_RenderBackend/renderBackend.h>
#include <TFE_Settings/settings.h>
#include "gl.h"
#include <assert.h>
#include <string>
#include <array>

namespace ShaderGL
{
	static const char* c_shaderAttrName[]=
	{
		"vtx_pos",   // ATTR_POS
		"vtx_nrm",   // ATTR_NRM
		"vtx_uv",    // ATTR_UV
		"vtx_uv1",   // ATTR_UV1
		"vtx_uv2",   // ATTR_UV2
		"vtx_uv3",   // ATTR_UV3
		"vtx_color", // ATTR_COLOR
	};

#if defined(ANDROID)
	static const GLchar* c_glslVersionString[] = { "#version 130\n", "#version 320 es\n", "#version 450\n" };
#else
	static const GLchar* c_glslVersionString[] = { "#version 130\n", "#version 330\n", "#version 450\n" };
#endif
	static std::vector<char> s_buffers[2];
	static std::string s_defineString;
	static std::string s_vertexFile, s_fragmentFile;

	// If you get an error please report on github. You may try different GL context version or GLSL version. See GL<>GLSL version table at the top of this file.
	bool CheckShader(GLuint handle, const char* desc)
	{
		GLint status = 0, log_length = 0;
		glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
		glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);
		TFE_ASSERT_GL;
		if ((GLboolean)status == GL_FALSE)
		{
			TFE_System::logWrite(LOG_ERROR, "Shader", "Failed to compile '%s'!\n", desc);
		}

		if (log_length > 1)
		{
			std::vector<char> buf;
			buf.resize(size_t(log_length + 1));
			buf.back() = 0;
			glGetShaderInfoLog(handle, log_length, NULL, (GLchar*)buf.data());
			TFE_ASSERT_GL;
			TFE_System::logWrite(LOG_ERROR, "Shader", "Error: %s\n", buf.data());
		}
		return (GLboolean)status == GL_TRUE;
	}

	// If you get an error please report on GitHub. You may try different GL context version or GLSL version.
	bool CheckProgram(GLuint handle, const char* desc, ShaderVersion version)
	{
		GLint status = 0, log_length = 0;
		glGetProgramiv(handle, GL_LINK_STATUS, &status);
		glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);
		TFE_ASSERT_GL;
		if ((GLboolean)status == GL_FALSE)
		{
			TFE_System::logWrite(LOG_ERROR, "Shader", "Failed to link '%s' - with GLSL %s", desc, c_glslVersionString[version]);
		}

		if (log_length > 1)
		{
			std::vector<char> buf;
			buf.resize(size_t(log_length + 1));
			buf.back() = 0;
			glGetShaderInfoLog(handle, log_length, NULL, (GLchar*)buf.data());
			TFE_ASSERT_GL;
			TFE_System::logWrite(LOG_ERROR, "Shader", "Error: %s\n", buf.data());
		}

		return (GLboolean)status == GL_TRUE;
	}
}


const char* vertexShaderDefine = "#define VERTEX_SHADER\n";
const char* fragmentShaderDefine = "#define FRAGMENT_SHADER\n";

const char* extension_GL_OES_standard_derivatives = "#extension GL_OES_standard_derivatives : enable\n";
const char* extension_GL_EXT_clip_cull_distance = "#extension GL_EXT_clip_cull_distance : enable\n";
const char* extension_GL_OVR_multiview2 = "#extension GL_OVR_multiview2 : enable\n";
const char* extension_GL_NV_shader_noperspective_interpolation = "#extension GL_NV_shader_noperspective_interpolation : enable\n";

const char* noperspective_interpolation = "#define NOPERSPECTIVE noperspective\n";
const char* no_noperspective_interpolation = "#define NOPERSPECTIVE\n";

#if defined(ANDROID)
const char* defaultPrecisions = R"(
	precision highp int;
	precision highp float;
	precision highp sampler2D;
	precision highp usampler2D;
	precision highp sampler3D;
	precision highp samplerCube;
	precision highp sampler2DArray;
	precision highp sampler2DShadow;
	precision highp samplerCubeShadow;
	precision highp sampler2DArrayShadow;
	precision highp samplerBuffer;
	precision highp isamplerBuffer;
	precision highp usamplerBuffer;
)";
#else
const char* defaultPrecisions = R"(
	#define highp
	#define mediump
	#define lowp
)";
#endif

bool Shader::create(const char* vertexShaderGLSL, const char* fragmentShaderGLSL, const char* defineString/* = nullptr*/, ShaderVersion version/* = SHADER_VER_COMPTABILE*/)
{
	if (version != ShaderVersion::SHADER_VER_STD)
	{
#if defined(ANDROID)
		TFE_ERROR("GL", "Only SHADER_VER_STD is supported.");
#endif
	}
	// Create shaders
	m_shaderVersion = version;

	u32 vertHandle = glCreateShader(GL_VERTEX_SHADER);
	TFE_ASSERT_GL;
	{
		std::vector<const GLchar*> vertex_shader_parts = { ShaderGL::c_glslVersionString[m_shaderVersion] };
		if (TFE_Settings::getTempSettings()->vr)
		{
			vertex_shader_parts.push_back(extension_GL_OVR_multiview2);
		}
#if defined(ANDROID)
		if (OpenGL_Caps::supportsClipping())
		{
			vertex_shader_parts.push_back(extension_GL_EXT_clip_cull_distance);
		}
		if (OpenGL_Caps::supportsNoPerspectiveInterpolation())
		{
			vertex_shader_parts.push_back(extension_GL_NV_shader_noperspective_interpolation);
			vertex_shader_parts.push_back(noperspective_interpolation);
		}
		else
		{
			vertex_shader_parts.push_back(no_noperspective_interpolation);
		}
#else
		vertex_shader_parts.push_back(noperspective_interpolation);
#endif
		vertex_shader_parts.push_back(vertexShaderDefine);
		vertex_shader_parts.push_back(defaultPrecisions);

		if (TFE_Settings::getTempSettings()->vr)
		{
			vertex_shader_parts.push_back("#define OPT_VR 1\r\n");
			if (TFE_Settings::getTempSettings()->vrMultiview)
			{
				vertex_shader_parts.push_back("#define OPT_VR_MULTIVIEW 1\r\n");
			}
		}

		if (defineString)
		{
			vertex_shader_parts.push_back(defineString);
		}
		vertex_shader_parts.push_back(vertexShaderGLSL);

		glShaderSource(vertHandle, (GLsizei)vertex_shader_parts.size(), vertex_shader_parts.data(), nullptr);
		glCompileShader(vertHandle);
		TFE_ASSERT_GL;
		if (!ShaderGL::CheckShader(vertHandle, ShaderGL::s_vertexFile.c_str()))
		{
			return false;
		}
	}

	u32 fragHandle = glCreateShader(GL_FRAGMENT_SHADER);
	TFE_ASSERT_GL;
	{
		std::vector<const GLchar*> fragment_shader_parts = { ShaderGL::c_glslVersionString[m_shaderVersion] };
		if (TFE_Settings::getTempSettings()->vr)
		{
			fragment_shader_parts.push_back(extension_GL_OVR_multiview2);
		}
#if defined(ANDROID)
		fragment_shader_parts.push_back(extension_GL_OES_standard_derivatives);
		if (OpenGL_Caps::supportsNoPerspectiveInterpolation())
		{
			fragment_shader_parts.push_back(extension_GL_NV_shader_noperspective_interpolation);
			fragment_shader_parts.push_back(noperspective_interpolation);
		}
		else
		{
			fragment_shader_parts.push_back(no_noperspective_interpolation);
		}
#else
		fragment_shader_parts.push_back(noperspective_interpolation);
#endif
		fragment_shader_parts.push_back(fragmentShaderDefine);
		fragment_shader_parts.push_back(defaultPrecisions);

		if (TFE_Settings::getTempSettings()->vr)
		{
			fragment_shader_parts.push_back("#define OPT_VR 1\r\n");
			if (TFE_Settings::getTempSettings()->vrMultiview)
			{
				fragment_shader_parts.push_back("#define OPT_VR_MULTIVIEW 1\r\n");
			}
		}

		if (defineString)
		{
			fragment_shader_parts.push_back(defineString);
		}
		fragment_shader_parts.push_back(fragmentShaderGLSL);
		glShaderSource(fragHandle, (GLsizei)fragment_shader_parts.size(), fragment_shader_parts.data(), nullptr);
		glCompileShader(fragHandle);
		TFE_ASSERT_GL;
		if (!ShaderGL::CheckShader(fragHandle, ShaderGL::s_fragmentFile.c_str()))
		{
			return false;
		}
	}

	m_gpuHandle = glCreateProgram();
	glAttachShader(m_gpuHandle, vertHandle);
	glAttachShader(m_gpuHandle, fragHandle);
	TFE_ASSERT_GL;
	// Bind vertex attribute names to slots.
	for (u32 i = 0; i < ATTR_COUNT; i++)
	{
		glBindAttribLocation(m_gpuHandle, i, ShaderGL::c_shaderAttrName[i]);
	}
	glLinkProgram(m_gpuHandle);
	TFE_ASSERT_GL;
	if (!ShaderGL::CheckProgram(m_gpuHandle, "shader program", m_shaderVersion))
	{
		return false;
	}

	{
		GLint numUniforms;
		glGetProgramiv(m_gpuHandle, GL_ACTIVE_UNIFORMS, &numUniforms);
		TFE_ASSERT_GL;

		std::array<GLchar, 512> nameBuffer;
		for (GLint i = 0; i < numUniforms; i++)
		{
			GLint size;
			GLenum typeGL;
			glGetActiveUniform(m_gpuHandle, i, (GLsizei)nameBuffer.size(), nullptr, &size, &typeGL, nameBuffer.data());
			TFE_ASSERT_GL;

			const GLint location = glGetUniformLocation(m_gpuHandle, nameBuffer.data());
			TFE_ASSERT_GL;

			m_uniforms.push_back({ nameBuffer.data(), location, size, typeGL });
		}
	}

	return m_gpuHandle != 0;
}

bool Shader::load(const char* vertexShaderFile, const char* fragmentShaderFile, u32 defineCount/* = 0*/, ShaderDefine* defines/* = nullptr*/, ShaderVersion version/* = SHADER_VER_COMPTABILE*/)
{
	m_shaderVersion = version;
	ShaderGL::s_buffers[0].clear();
	ShaderGL::s_buffers[1].clear();

	GLSLParser::parseFile(vertexShaderFile,   ShaderGL::s_buffers[0]);
	GLSLParser::parseFile(fragmentShaderFile, ShaderGL::s_buffers[1]);

	ShaderGL::s_vertexFile = vertexShaderFile;
	ShaderGL::s_fragmentFile = fragmentShaderFile;

	ShaderGL::s_buffers[0].push_back(0);
	ShaderGL::s_buffers[1].push_back(0);

	// Build a string of defines.
	ShaderGL::s_defineString.clear();

	// first add shader files so we can see it in NSight/RenderDoc
	ShaderGL::s_defineString += fmt::format("// vs: \"{}\"\r\n", vertexShaderFile);
	ShaderGL::s_defineString += fmt::format("// fs: \"{}\"\r\n", fragmentShaderFile);

	if (defineCount)
	{
		ShaderGL::s_defineString += "\r\n";
		for (u32 i = 0; i < defineCount; i++)
		{
			ShaderGL::s_defineString += "#define ";
			ShaderGL::s_defineString += defines[i].name;
			ShaderGL::s_defineString += " ";
			ShaderGL::s_defineString += defines[i].value;
			ShaderGL::s_defineString += "\r\n";
		}
		ShaderGL::s_defineString += "\r\n";
	}

	const bool b = create(ShaderGL::s_buffers[0].data(), ShaderGL::s_buffers[1].data(), ShaderGL::s_defineString.c_str(), m_shaderVersion);
	if (!b)
	{
		TFE_ERROR("GL", "compiling shader vs={}, fs={}", vertexShaderFile, fragmentShaderFile);
	}
	return b;
}

void Shader::enableClipPlanes(s32 count)
{
	m_clipPlaneCount = count;
}

void Shader::destroy()
{
	if (m_gpuHandle)
	{
		glDeleteProgram(m_gpuHandle);
	}
	m_gpuHandle = 0;
}

void Shader::bind()
{
	glUseProgram(m_gpuHandle);
	TFE_ASSERT_GL;
	TFE_RenderState::enableClipPlanes(m_clipPlaneCount);
}

void Shader::unbind()
{
	glUseProgram(0);
	TFE_ASSERT_GL;
}

s32 Shader::getVariableId(const char* name, const ShaderUniform** uniform)
{
	s32 location = glGetUniformLocation(m_gpuHandle, name);
	if (location < 0)
	{
		const std::string name_ = fmt::format("{}_", name);
		location = glGetUniformLocation(m_gpuHandle, name_.data());
		if (location >= 0)
		{
			std::ignore = 5;
		}
	}
	if (location >= 0 && uniform)
	{
		if (const auto& it = std::find_if(m_uniforms.begin(), m_uniforms.end(), [location](const ShaderUniform& v) { return v.location == location; }); it != m_uniforms.end())
		{
			const ShaderUniform* addr = &(*it);
			if (addr->size > 1)
			{
				std::ignore = 5;
			}
			*uniform = addr;
		}
		else
		{
			*uniform = nullptr;
			TFE_ERROR("Shader", "Uniform not found: {}", name);
		}
	}
	return location;
}

// For debugging.
s32 Shader::getVariables()
{
	s32 length;
	s32 size;
	GLenum type;
	char name[256];

	s32 count;
	glGetProgramiv(m_gpuHandle, GL_ACTIVE_UNIFORMS, &count);
	printf("Active Uniforms: %d\n", count);

	for (s32 i = 0; i < count; i++)
	{
		glGetActiveUniform(m_gpuHandle, (GLuint)i, 256, &length, &size, &type, name);
		printf("Uniform #%d Type: %u Name: %s\n", i, type, name);
	}

	s32 attribCount;
	glGetProgramiv(m_gpuHandle, GL_ACTIVE_ATTRIBUTES, &attribCount);
	printf("Active Attributes: %d\n", attribCount);

	for (s32 i = 0; i < attribCount; i++)
	{
		glGetActiveAttrib(m_gpuHandle, (GLuint)i, 256, &length, &size, &type, name);
		printf("Attribute #%d Type: %u Name: %s\n", i, type, name);
	}
	TFE_ASSERT_GL;

	return count;
}

void Shader::bindTextureNameToSlot(const char* texName, s32 slot)
{
	const s32 curSlot = glGetUniformLocation(m_gpuHandle, texName);
	if (curSlot < 0 || slot < 0) { return; }

	bind();
	glUniform1i(curSlot, slot);
	TFE_ASSERT_GL;
	unbind();
}

void Shader::setVariable(s32 id, ShaderVariableType type, const f32* data)
{
	if (id < 0) { return; }

	switch (type)
	{
	case SVT_SCALAR:
		glUniform1f(id, data[0]);
		break;
	case SVT_VEC2:
		glUniform2fv(id, 1, data);
		break;
	case SVT_VEC3:
		glUniform3fv(id, 1, data);
		break;
	case SVT_VEC4:
		glUniform4fv(id, 1, data);
		break;
	case SVT_MAT3x3:
		glUniformMatrix3fv(id, 1, false, data);
		break;
	case SVT_MAT4x3:
		glUniformMatrix4x3fv(id, 1, false, data);
		break;
	case SVT_MAT4x4:
		glUniformMatrix4fv(id, 1, false, data);
		break;
	default:
		TFE_System::logWrite(LOG_ERROR, "Shader", "Mismatched parameter type.");
		assert(0);
	}

	TFE_ASSERT_GL;
}

void Shader::setVariableArray(s32 id, ShaderVariableType type, const f32* data, u32 count)
{
	if (id < 0) { return; }

	switch (type)
	{
	case SVT_SCALAR:
		glUniform1fv(id, count, data);
		break;
	case SVT_VEC2:
		glUniform2fv(id, count, data);
		break;
	case SVT_VEC3:
		glUniform3fv(id, count, data);
		break;
	case SVT_VEC4:
		glUniform4fv(id, count, data);
		break;
	case SVT_MAT3x3:
		glUniformMatrix3fv(id, count, false, data);
		break;
	case SVT_MAT4x3:
		glUniformMatrix4x3fv(id, count, false, data);
		break;
	case SVT_MAT4x4:
		glUniformMatrix4fv(id, count, false, data);
		break;
	default:
		TFE_System::logWrite(LOG_ERROR, "Shader", "Mismatched parameter type.");
		assert(0);
	}

	TFE_ASSERT_GL;
}

void Shader::setVariable(s32 id, ShaderVariableType type, const s32* data)
{
	if (id < 0) { return; }

	switch (type)
	{
	case SVT_ISCALAR:
		glUniform1i(id, *(&data[0]));
		break;
	case SVT_IVEC2:
		glUniform2iv(id, 1, data);
		break;
	case SVT_IVEC3:
		glUniform3iv(id, 1, data);
		break;
	case SVT_IVEC4:
		glUniform4iv(id, 1, data);
		break;
	default:
		TFE_System::logWrite(LOG_ERROR, "Shader", "Mismatched parameter type.");
		assert(0);
	}

	TFE_ASSERT_GL;
}

void Shader::setVariable(s32 id, ShaderVariableType type, const u32* data)
{
	if (id < 0) { return; }

	switch (type)
	{
	case SVT_USCALAR:
		glUniform1ui(id, *(&data[0]));
		break;
	case SVT_UVEC2:
		glUniform2uiv(id, 1, data);
		break;
	case SVT_UVEC3:
		glUniform3uiv(id, 1, data);
		break;
	case SVT_UVEC4:
		glUniform4uiv(id, 1, data);
		break;
	default:
		TFE_System::logWrite(LOG_ERROR, "Shader", "Mismatched parameter type.");
		assert(0);
	}

	TFE_ASSERT_GL;
}
