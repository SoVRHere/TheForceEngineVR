#include "renderTarget.h"
#include <TFE_RenderBackend/renderState.h>
#include <TFE_RenderBackend/renderBackend.h>
#include <TFE_RenderBackend/Win32OpenGL/openGL_Debug.h>
#include <assert.h>
#include <tuple>

namespace
{
	u32 createDepthBuffer(u32 width, u32 height)
	{
		u32 handle = 0;
		glGenRenderbuffers(1, &handle);
		glBindRenderbuffer(GL_RENDERBUFFER, handle);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		TFE_ASSERT_GL;

		return handle;
	}
}

RenderTarget::~RenderTarget()
{
	glDeleteFramebuffers(1, &m_gpuHandle);
	TFE_ASSERT_GL;
	m_gpuHandle = 0;

	if (m_depthBufferHandle)
	{
		glDeleteRenderbuffers(1, &m_depthBufferHandle);
		TFE_ASSERT_GL;
		m_depthBufferHandle = 0;
	}
}

bool RenderTarget::create(s32 textureCount, TextureGpu** textures, bool depthBuffer)
{
	if (textureCount < 1 || !textures || !textures[0]) { return false; }
	m_textureCount = textureCount;
	for (u32 i = 0; i < m_textureCount; i++)
	{
		m_texture[i] = textures[i];
	}

	glGenFramebuffers(1, &m_gpuHandle);
	glBindFramebuffer(GL_FRAMEBUFFER, m_gpuHandle);
	for (u32 i = 0; i < m_textureCount; i++)
	{
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, m_texture[i]->getHandle(), 0);
	}

	m_depthBufferHandle = 0;
	if (depthBuffer)
	{
		m_depthBufferHandle = createDepthBuffer(m_texture[0]->getWidth(), m_texture[0]->getHeight());
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthBufferHandle);
	}

	// Set the list of draw buffers.
	GLenum drawBuffers[MAX_ATTACHMENT] = { 0 };
	for (u32 i = 0; i < m_textureCount; i++)
	{
		drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
	}
	glDrawBuffers(m_textureCount, drawBuffers);

	OpenGL_Debug::CheckRenderTargetStatus(GL_FRAMEBUFFER);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	TFE_ASSERT_GL;
	return true;
}

bool RenderTarget::create(s32 textureCount, TextureGpu** textures, TextureGpu* depthTexture, bool multiview)
{
	if (textureCount < 1 || !textures || !textures[0]) {
		return false;
	}
	m_textureCount = textureCount;
	for (u32 i = 0; i < m_textureCount; i++)
	{
		m_texture[i] = textures[i];
	}
	m_depth = depthTexture;

	glGenFramebuffers(1, &m_gpuHandle);
	glBindFramebuffer(GL_FRAMEBUFFER, m_gpuHandle);

	// colors
	if (multiview)
	{
		for (u32 i = 0; i < m_textureCount; i++)
		{
			glFramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, m_texture[i]->getHandle(), 0, 0, 2);
		}
	}
	else
	{
		for (u32 i = 0; i < m_textureCount; i++)
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, m_texture[i]->getHandle(), 0);
		}
	}
	TFE_ASSERT_GL;

	// depth
	if (m_depth)
	{
		if (multiview)
		{
			glFramebufferTextureMultiviewOVR(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, m_depth->getHandle(), 0, 0, 2);
		}
		else
		{
			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, m_depth->getHandle(), 0);
		}
	}
	TFE_ASSERT_GL;

	mMultiview = multiview;

	// Set the list of draw buffers.
	GLenum drawBuffers[MAX_ATTACHMENT] = { 0 };
	for (u32 i = 0; i < m_textureCount; i++)
	{
		drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
	}
	glDrawBuffers(m_textureCount, drawBuffers);
	TFE_ASSERT_GL;

	if (!OpenGL_Debug::CheckRenderTargetStatus(GL_FRAMEBUFFER))
	{
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	TFE_ASSERT_GL;

	return true;
}

void RenderTarget::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_gpuHandle);
	glViewport(0, 0, m_texture[0]->getWidth(), m_texture[0]->getHeight());
	glDepthRangef(0.0f, 1.0f);
	TFE_ASSERT_GL;

	OpenGL_Debug::CheckRenderTargetStatus(GL_FRAMEBUFFER);
}

void RenderTarget::invalidate()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_gpuHandle);
	GLsizei numAttachments = 0;
	GLenum attachments[8];
	for (u32 i = 0; i < m_textureCount; i++)
	{
		attachments[numAttachments++] = GL_COLOR_ATTACHMENT0 + i;
	}
	if (m_depthBufferHandle || m_depth)
	{
		attachments[numAttachments++] = GL_DEPTH_STENCIL_ATTACHMENT;
	}
	glInvalidateFramebuffer(GL_FRAMEBUFFER, numAttachments, attachments);
	TFE_ASSERT_GL;
}

void RenderTarget::clear(const f32* color, f32 depth, u8 stencil, bool clearColor)
{
	OpenGL_Debug::CheckRenderTargetStatus(GL_FRAMEBUFFER);

	GLint buffer;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &buffer);
	TFE_ASSERT_GL;
	if (buffer == 0)
	{
		std::ignore = 5;
	}

	if (color)
		glClearColor(color[0], color[1], color[2], color[3]);
	else
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	u32 clearFlags = clearColor ? GL_COLOR_BUFFER_BIT : 0;
	if (m_depthBufferHandle || m_depth)
	{
		clearFlags |= GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
		TFE_RenderState::setStateEnable(true, STATE_DEPTH_WRITE | STATE_STENCIL_WRITE);
		glClearDepthf(depth);
		glClearStencil(stencil);
		TFE_ASSERT_GL;
	}

	glClear(clearFlags);
	TFE_ASSERT_GL;
}

 void RenderTarget::clearDepth(f32 depth)
 {
	 if (m_depthBufferHandle || m_depth)
	 {
		 OpenGL_Debug::CheckRenderTargetStatus(GL_FRAMEBUFFER);

		 TFE_RenderState::setStateEnable(true, STATE_DEPTH_WRITE);
		 glClearDepthf(depth);
		 glClear(GL_DEPTH_BUFFER_BIT);
		 TFE_ASSERT_GL;
	 }
 }

 void RenderTarget::clearStencil(u8 stencil)
 {
	 if (m_depthBufferHandle || m_depth)
	 {
		 OpenGL_Debug::CheckRenderTargetStatus(GL_FRAMEBUFFER);

		 TFE_RenderState::setStateEnable(true, STATE_STENCIL_WRITE);
		 glClearStencil(stencil);
		 glClear(GL_STENCIL_BUFFER_BIT);
		 TFE_ASSERT_GL;
	 }
 }

void RenderTarget::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	TFE_ASSERT_GL;
}

void RenderTarget::copy(RenderTarget* dst, RenderTarget* src)
{
	OpenGL_Debug::CheckRenderTargetStatus(GL_FRAMEBUFFER);

	bool multiview = src->mMultiview;

	if (multiview)
	{
		for (int view = 0; view < 2; view++)
		{
			// Bind the source and destination framebuffers
			glBindFramebuffer(GL_READ_FRAMEBUFFER, src->m_gpuHandle);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst->m_gpuHandle);
			TFE_ASSERT_GL;

			// Specify the layer of the texture to read from and draw to
			glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, src->getTexture()->getHandle()/*sourceTextureArray*/, 0, view);
			glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, dst->getTexture()->getHandle()/*destinationTextureArray*/, 0, view);
			TFE_ASSERT_GL;

			// Blit the framebuffer for the current view
			glBlitFramebuffer(0, 0, src->getTexture()->getWidth(), src->getTexture()->getHeight(),	// src rect
				0, 0, dst->getTexture()->getWidth(), dst->getTexture()->getHeight(),	// dst rect
				GL_COLOR_BUFFER_BIT, GL_NEAREST);
			TFE_ASSERT_GL;
		}
	}
	else
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, src->m_gpuHandle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst->m_gpuHandle);
		TFE_ASSERT_GL;

		glBlitFramebuffer(0, 0, src->getTexture()->getWidth(), src->getTexture()->getHeight(),	// src rect
			0, 0, dst->getTexture()->getWidth(), dst->getTexture()->getHeight(),	// dst rect
			GL_COLOR_BUFFER_BIT, GL_NEAREST);
		TFE_ASSERT_GL;

		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		TFE_ASSERT_GL;
	}
}

void RenderTarget::copyBackbufferToTarget(RenderTarget* dst)
{
	OpenGL_Debug::CheckRenderTargetStatus(GL_FRAMEBUFFER);

	glReadBuffer(GL_BACK);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst->m_gpuHandle);
	TFE_ASSERT_GL;

	DisplayInfo displayInfo;
	TFE_RenderBackend::getDisplayInfo(&displayInfo);
	
	glBlitFramebuffer(0, displayInfo.height, displayInfo.width, 0,	// src rect
		0, 0, dst->getTexture()->getWidth(), dst->getTexture()->getHeight(),	// dst rect
		GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	TFE_ASSERT_GL;
}
