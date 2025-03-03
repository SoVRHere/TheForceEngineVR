#include "openGL_Debug.h"
#include <TFE_System/system.h>

namespace OpenGL_Debug
{
	void AssertGL(const char* file, int line)
	{
#define DOCASE(error) \
	case error: \
	errorTxt = #error; \
	break

		GLenum err;
		while ((err = glGetError()) != GL_NO_ERROR)
		{
			const char* errorTxt;

			switch (err)
			{
				DOCASE(GL_INVALID_ENUM);
				DOCASE(GL_INVALID_VALUE);
				DOCASE(GL_INVALID_OPERATION);
#if defined(GL_STACK_OVERFLOW)
				DOCASE(GL_STACK_OVERFLOW);
#endif
#if defined(GL_STACK_UNDERFLOW)
				DOCASE(GL_STACK_UNDERFLOW);
#endif
				DOCASE(GL_OUT_OF_MEMORY);
				DOCASE(GL_INVALID_FRAMEBUFFER_OPERATION);

			default:
				errorTxt = "(unknown)";
				break;
			}

			TFE_ERROR("OpenGL", "AssertGL: {} (0x{:X}) file:{} line:{}", errorTxt, err, file, line);
		}

#undef DOCASE
	}

	const char* GetConstStr(GLenum constant)
	{
#define DOCASE(v) case v: return #v

		switch (constant)
		{
			DOCASE(GL_NONE);

			//DOCASE(GL_RGB4);
			//DOCASE(GL_RGB5);
			DOCASE(GL_RGB8);
			//DOCASE(GL_RGB10);
			//DOCASE(GL_RGB12);
			DOCASE(GL_RGB16);
			DOCASE(GL_RGB16_SNORM);
			DOCASE(GL_RGB16F);
			DOCASE(GL_R11F_G11F_B10F);

			DOCASE(GL_RGBA4);
			DOCASE(GL_RGBA8);
			DOCASE(GL_RGB10_A2);
			//DOCASE(GL_RGBA12);
			DOCASE(GL_RGBA16);
			DOCASE(GL_RGBA8_SNORM);
			DOCASE(GL_RGBA16_SNORM);
			DOCASE(GL_RGBA16F);
			DOCASE(GL_SRGB8_ALPHA8);

			DOCASE(GL_DEPTH_COMPONENT16);
			DOCASE(GL_DEPTH_COMPONENT24);
			//DOCASE(GL_DEPTH_COMPONENT32);
			DOCASE(GL_DEPTH24_STENCIL8);
			DOCASE(GL_DEPTH_COMPONENT32F);
			DOCASE(GL_DEPTH32F_STENCIL8);

		default:
		{
			static std::string constStr;
			constStr = fmt::format("0x{:X}", constant);
			TFE_WARN("GL", "unknown OpenGL const {}", constStr);
			return constStr.c_str();
		}
		}
#undef DOCASE
	}

	bool CheckRenderTargetStatus(GLuint target)
	{
#ifdef _DEBUG
		const GLenum status = glCheckFramebufferStatus(target);
		TFE_ASSERT_GL;

		if (status == GL_FRAMEBUFFER_COMPLETE)
		{
			return true;
		}

		switch (status)
		{
		case GL_FRAMEBUFFER_COMPLETE:
			return true;

		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			TFE_ERROR("GL", "CheckRenderTargetStatus: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER.");
			return true;

		case GL_FRAMEBUFFER_UNSUPPORTED:
			TFE_ERROR("GL", "CheckRenderTargetStatus: GL_FRAMEBUFFER_UNSUPPORTED. Choose different formats.");
			return false;

		case GL_FRAMEBUFFER_UNDEFINED:
			TFE_ERROR("GL", "CheckRenderTargetStatus: GL_FRAMEBUFFER_UNDEFINED.");
			return false;

		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			TFE_ERROR("GL", "CheckRenderTargetStatus: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT.");
			return false;

		//case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
		//	TFE_ERROR("GL", "CheckRenderTargetStatus: GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS.");
		//	return false;

		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			TFE_ERROR("GL", "CheckRenderTargetStatus: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT.");
			return false;

		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			TFE_ERROR("GL", "CheckRenderTargetStatus: GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER.");
			return false;

		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			TFE_ERROR("GL", "CheckRenderTargetStatus: GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE.");
			return false;

		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			TFE_ERROR("GL",
				"CheckRenderTargetStatus: GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS. You are probably mixing "
				"layered texture attachments (e.g. Texture2DArray) with and nonlayered texture attachments (e.g. Texture2D)");
			return false;

#ifdef GL_FRAMEBUFFER_INCOMPLETE_VIEW_TARGETS_OVR
		case GL_FRAMEBUFFER_INCOMPLETE_VIEW_TARGETS_OVR:
			TFE_ERROR("GL", "CheckRenderTargetStatus: GL_FRAMEBUFFER_INCOMPLETE_VIEW_TARGETS_OVR.");
			return false;
#endif

		default:
			TFE_ERROR("GL", "CheckRenderTargetStatus: {}. Programming error. This will fail on all hardware.", status);
			return false;
		}
#else
		return true;
#endif
	}

	GLenum ObjectTypeToObjectTypeGL(const GLObjectType type)
	{
		static const GLenum types[size_t(GLObjectType::Count)] = {
			GL_SHADER,
			GL_PROGRAM,
			GL_BUFFER,
			GL_TEXTURE,
			GL_FRAMEBUFFER
		};

		return types[size_t(type)];
	}

	void SetObjectName(GLObjectType identifier, GLuint id, const char* name)
	{
		const GLsizei len = (GLsizei)std::strlen(name); // TODO: limit by GL_MAX_LABEL_LENGTH
		glObjectLabel(ObjectTypeToObjectTypeGL(identifier), id, len, name);
		TFE_ASSERT_GL;
	}

	void PushGroup(const char* label)
	{
		const GLsizei len = (GLsizei)std::strlen(label); // TODO: limit by GL_MAX_DEBUG_MESSAGE_LENGTH
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, len, label);
		TFE_ASSERT_GL;
	}

	void PopGroup()
	{
		glPopDebugGroup();
		TFE_ASSERT_GL;
	}

	const char* DebugMsgSourceToString(GLenum source)
	{
#define DOCASE(v) case v: return #v
		switch (source)
		{
			DOCASE(GL_DEBUG_SOURCE_API);
			DOCASE(GL_DEBUG_SOURCE_WINDOW_SYSTEM);
			DOCASE(GL_DEBUG_SOURCE_SHADER_COMPILER);
			DOCASE(GL_DEBUG_SOURCE_THIRD_PARTY);
			DOCASE(GL_DEBUG_SOURCE_APPLICATION);
			DOCASE(GL_DEBUG_SOURCE_OTHER);
		default:
			TFE_ERROR("OpenGL", "Unknown debug message source {}", source);
			return "";
		};
#undef DOCASE
	}

	const char* DebugMsgTypeToString(GLenum type)
	{
#define DOCASE(v) case v: return #v
		switch (type)
		{
			DOCASE(GL_DEBUG_TYPE_ERROR);
			DOCASE(GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR);
			DOCASE(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR);
			DOCASE(GL_DEBUG_TYPE_PORTABILITY);
			DOCASE(GL_DEBUG_TYPE_PERFORMANCE);
			DOCASE(GL_DEBUG_TYPE_MARKER);
			DOCASE(GL_DEBUG_TYPE_PUSH_GROUP);
			DOCASE(GL_DEBUG_TYPE_POP_GROUP);
			DOCASE(GL_DEBUG_TYPE_OTHER);
		default:
			TFE_ERROR("OpenGL", "Unknown debug message type {}", type);
			return "";
		};
#undef DOCASE
	}

	const char* DebugMsgSeverityToString(GLenum severity)
	{
#define DOCASE(v) case v: return #v
		switch (severity)
		{
			DOCASE(GL_DEBUG_SEVERITY_HIGH);
			DOCASE(GL_DEBUG_SEVERITY_MEDIUM);
			DOCASE(GL_DEBUG_SEVERITY_LOW);
			DOCASE(GL_DEBUG_SEVERITY_NOTIFICATION);
		default:
			TFE_ERROR("OpenGL", "Unknown debug message severity {}", severity);
			return "";
		};
#undef DOCASE
	}

#if defined(_WIN32)
#define DEBUGCALLBACK_CALLING_CONVENSION __stdcall
#else
#define DEBUGCALLBACK_CALLING_CONVENSION
#endif
	void DEBUGCALLBACK_CALLING_CONVENSION DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:
			TFE_ERROR("OpenGL", "(source = {}, type = {}, id = {}, severity = {}) : '{}'", DebugMsgSourceToString(source), DebugMsgTypeToString(type), id, DebugMsgSeverityToString(severity), message);
			break;
		case GL_DEBUG_SEVERITY_MEDIUM:
			TFE_WARN("OpenGL", "(source={}, type={}, id={}, severity={}): '{}'", DebugMsgSourceToString(source), DebugMsgTypeToString(type), id, DebugMsgSeverityToString(severity), message);
			break;
		case GL_DEBUG_SEVERITY_LOW:
		default:
			TFE_INFO("OpenGL", "(source={}, type={}, id={}, severity={}): '{}'", DebugMsgSourceToString(source), DebugMsgTypeToString(type), id, DebugMsgSeverityToString(severity), message);
			break;
		};
	}

	void Initialize()
	{
#ifdef _DEBUG
		if (glDebugMessageCallback)
		{
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			TFE_ASSERT_GL;

			glDebugMessageCallback(DebugCallback, nullptr);
			TFE_ASSERT_GL;

			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_TRUE);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
			TFE_ASSERT_GL;

			//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_TRUE);
			//TFE_ASSERT_GL;
			//glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_TRUE);
			//TFE_ASSERT_GL;
		}
#endif
	}
}
