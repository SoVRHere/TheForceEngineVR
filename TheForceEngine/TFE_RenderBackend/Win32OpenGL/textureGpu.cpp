#include <TFE_RenderBackend/textureGpu.h>
#include <TFE_RenderBackend/Win32OpenGL/openGL_Debug.h>
#include <TFE_System/system.h>
#include <TFE_Settings/settings.h>
#include "openGL_Caps.h"
#include "gl.h"
#include <algorithm>
#include <vector>
#include <assert.h>

static std::vector<u8> s_workBuffer;
const GLenum c_channelFormat[] = { GL_UNSIGNED_BYTE, GL_UNSIGNED_BYTE, GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_UNSIGNED_INT_24_8 };
const GLenum c_internalFormat[] = { GL_RGBA8, GL_R8, GL_RGBA16F, GL_R11F_G11F_B10F, GL_R16F, GL_DEPTH24_STENCIL8 };
const GLenum c_baseFormat[] = { GL_RGBA, GL_RED, GL_RGBA, GL_RGB, GL_RED, GL_DEPTH_STENCIL };
const GLenum c_channelCount[] = { 4, 1, 4, 1, 1 };
const GLenum c_bytesPerChannel[] = {1, 1, 2, 2, 4 };

static const char* c_magFilterStr[] =
{
	"MAG_FILTER_NONE",
	"MAG_FILTER_LINEAR",
};

static const char* c_texFormatStr[] =
{
	"TEX_RGBA8",
	"TEX_R8",
	"TEX_RGBAF16",
	"TEX_R16F",
	"TEX_DEPTH24_STENCIL8"
};

TextureGpu::~TextureGpu()
{
	if (m_gpuHandle && m_handleOwner)
	{
		glDeleteTextures(1, &m_gpuHandle);
		TFE_ASSERT_GL;
	}
	m_gpuHandle = 0;
}

bool TextureGpu::create(u32 width, u32 height, TexFormat format, bool hasMipmaps, MagFilter magFilter)
{
	m_width = width;
	m_height = height;
	m_channels = c_channelCount[format];
	m_bytesPerChannel = c_bytesPerChannel[format];
	m_layers = 1;

	if (!m_handleOwner)
	{
		TFE_ASSERT(m_gpuHandle != 0);
		return true;
	}

	glGenTextures(1, &m_gpuHandle);
	if (!m_gpuHandle) { return false; }

	glBindTexture(GL_TEXTURE_2D, m_gpuHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, c_internalFormat[format], width, height, 0, c_baseFormat[format], c_channelFormat[format], nullptr);
	TFE_ASSERT_GL;
	//error = glGetError();
	// Handle OpenGL driver wonkiness around float16 textures.
	//if (error != GL_NO_ERROR && c_channelFormat[format] == GL_FLOAT)
	//{
	//	glTexImage2D(GL_TEXTURE_2D, 0, c_internalFormat[format], width, height, 0, c_baseFormat[format], GL_HALF_FLOAT, nullptr);
	//	error = glGetError();
	//}
	//assert(error == GL_NO_ERROR);
	//if (error != GL_NO_ERROR)
	//{
	//	TFE_System::logWrite(LOG_ERROR, "TextureGPU - OpenGL", "Failed to create texture - size: (%u, %u), format: %s. Error ID: 0x%x",
	//		m_width, m_height, c_texFormatStr[format], error);
	//	glBindTexture(GL_TEXTURE_2D, 0);
	//	return false;
	//}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, hasMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter == MAG_FILTER_LINEAR ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);
	TFE_ASSERT_GL;

	return true;
}

bool TextureGpu::createArray(u32 width, u32 height, u32 layers, u32 channels, u32 mipCount)
{
	// No need to make this a texture array if the layer count == 1.
	if (layers <= 1)
	{
		return create(width, height, channels == 4 ? TexFormat::TEX_RGBA8 : TexFormat::TEX_R8);
	}

	m_width = width;
	m_height = height;
	m_channels = channels;
	m_bytesPerChannel = 1;
	m_mipCount = mipCount;
	m_layers = layers;

	glGenTextures(1, &m_gpuHandle);
	if (!m_gpuHandle) { return false; }

	glBindTexture(GL_TEXTURE_2D_ARRAY, m_gpuHandle);
	for (u32 mip = 0; mip < m_mipCount; mip++)
	{
		if (channels == 1)
		{
			glTexImage3D(GL_TEXTURE_2D_ARRAY, mip, GL_R8, width, height, layers, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
		}
		else if (channels == 4)
		{
			glTexImage3D(GL_TEXTURE_2D_ARRAY, mip, GL_RGBA8, width, height, layers, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		}
		width  >>= 1;
		height >>= 1;
		TFE_ASSERT_GL;
	}

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	TFE_ASSERT_GL;

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	TFE_ASSERT_GL;
	return true;
}

bool TextureGpu::createArray(u32 width, u32 height, u32 layers, TexFormat format, bool hasMipmaps, MagFilter magFilter)
{
	m_width = width;
	m_height = height;
	m_channels = c_channelCount[format];
	m_bytesPerChannel = c_bytesPerChannel[format];
	m_mipCount = 1;
	m_layers = layers;

	if (!m_handleOwner)
	{
		TFE_ASSERT(m_gpuHandle != 0);
		return true;
	}

	glGenTextures(1, &m_gpuHandle);
	if (!m_gpuHandle) { return false; }

	glBindTexture(GL_TEXTURE_2D_ARRAY, m_gpuHandle);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, c_internalFormat[format], width, height, layers);
	TFE_ASSERT_GL;

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, hasMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, magFilter == MAG_FILTER_LINEAR ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	const GLfloat borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, borderColor);
	TFE_ASSERT_GL;

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	TFE_ASSERT_GL;
	return true;
}

bool TextureGpu::createWithData(u32 width, u32 height, const void* buffer, MagFilter magFilter)
{
	m_width = width;
	m_height = height;
	m_channels = 4;
	m_bytesPerChannel = 1;
	m_layers = 1;

	glGenTextures(1, &m_gpuHandle);
	if (!m_gpuHandle) { return false; }

	glBindTexture(GL_TEXTURE_2D, m_gpuHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	TFE_ASSERT_GL;
	
	f32 maxAniso = OpenGL_Caps::getMaxAnisotropy();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter == MAG_FILTER_NONE ? GL_NEAREST : GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	if (OpenGL_Caps::supportsAniso())
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, maxAniso);
	}
	TFE_ASSERT_GL;

	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	TFE_ASSERT_GL;

	return true;
}

bool TextureGpu::update(const void* buffer, size_t size, s32 layer, s32 mipLevel)
{
	s32 layerCount = layer < 0 ? m_layers : 1;
	s32 layerIndex = layer < 0 ? 0 : layer;
	//if (mipLevel == 0 && size < m_width * m_height * m_channels * layerCount) { return false; }

	u32 width  = m_width  >> mipLevel;
	u32 height = m_height >> mipLevel;

	if (m_layers == 1)
	{
		glBindTexture(GL_TEXTURE_2D, m_gpuHandle);
		glTexSubImage2D(GL_TEXTURE_2D, mipLevel, 0, 0, width, height, m_channels == 4 ? GL_RGBA : GL_RED, GL_UNSIGNED_BYTE, buffer);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_gpuHandle);
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, mipLevel, 0/*xOffset*/, 0/*yOffset*/, layerIndex, width, height, layerCount,
			m_channels == 4 ? GL_RGBA : GL_RED, GL_UNSIGNED_BYTE, buffer);
		glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	}

	TFE_ASSERT_GL;
	return true;
}

void TextureGpu::setFilter(MagFilter magFilter, MinFilter minFilter, bool isArray) const
{
	glTexParameteri(isArray ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter == MAG_FILTER_LINEAR ? GL_LINEAR : GL_NEAREST);
	if (minFilter == MIN_FILTER_MIPMAP && m_mipCount > 1)
	{
		const f32 ani = std::min(16.0f, OpenGL_Caps::getAnisotropyFromQuality(TFE_Settings::getGraphicsSettings()->anisotropyQuality));
		// Anisotropic filters read further into the mipchain than trilinear, so we have to factor that in and modify the maximum LOD to compensate.
		const f32 maxLodBias = (ani - 1.0f) * 0.2f;

		glTexParameteri(isArray ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(isArray ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, (s32)m_mipCount - 1);
		glTexParameterf(isArray ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, max(0.0f, (f32)m_mipCount - 1.0f - maxLodBias));
		if (OpenGL_Caps::supportsAniso())
		{
			glTexParameterf(isArray ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, ani);
		}
		TFE_ASSERT_GL;
	}
	else
	{
		glTexParameteri(isArray ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter == MIN_FILTER_NONE ? GL_NEAREST : GL_LINEAR);
		glTexParameteri(isArray ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameterf(isArray ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 0.0f);
		if (OpenGL_Caps::supportsAniso())
		{
			glTexParameterf(isArray ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 1.0f);
		}
		TFE_ASSERT_GL;
	}
}

void TextureGpu::bind(u32 slot/* = 0*/) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	if (m_layers == 1)
	{
		glBindTexture(GL_TEXTURE_2D, m_gpuHandle);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D_ARRAY, m_gpuHandle);
	}
	TFE_ASSERT_GL;
}

void TextureGpu::clear(u32 slot/* = 0*/)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, 0);
	TFE_ASSERT_GL;
}

void TextureGpu::clearSlots(u32 count, u32 start/* = 0*/)
{
	for (u32 i = 0; i < count; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i + start);
		glBindTexture(GL_TEXTURE_2D, 0);
		TFE_ASSERT_GL;
	}
}

void TextureGpu::readCpu(u8* image)
{
	glBindTexture(GL_TEXTURE_2D, m_gpuHandle);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glBindTexture(GL_TEXTURE_2D, 0);
	TFE_ASSERT_GL;
}
