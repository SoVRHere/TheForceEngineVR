#include "bloomDownsample.h"
#include "postprocess.h"
#include <TFE_RenderBackend/renderBackend.h>
#include <TFE_System/profiler.h>

bool BloomDownsample::init()
{
	return buildShaders();
}

void BloomDownsample::destroy()
{
	m_shaderInternal.destroy();
}

bool BloomDownsample::buildShaders()
{
	ShaderDefine defines[1];
	defines[0].name = "OPT_OPTIMIZED";
	defines[0].value = "1";

	// Base shader.
	if (!m_shaderInternal.load("Shaders/bloom.vert", "Shaders/bloomDownsample.frag", m_optimized ? 1 : 0, defines, SHADER_VER_STD))
	{
		return false;
	}
	m_shaderInternal.bindTextureNameToSlot("ColorBuffer", 0);
	setupShader();
	return true;
}

void BloomDownsample::setEffectState()
{
	TFE_RenderState::setStateEnable(false, STATE_CULLING | STATE_BLEND | STATE_DEPTH_TEST);

	if (inputCount == 1)
	{
		if (m_texSizeId >= 0)
		{
			Vec2f texSize{ (f32)inputs[0].tex->getWidth(), (f32)inputs[0].tex->getHeight() };
			m_shader->setVariable(m_texSizeId, SVT_VEC2, &texSize.x);
		}
	}
	else
	{
		TFE_ERROR("postprocess", "invalid input count");
	}
}

void BloomDownsample::setupShader()
{
	m_shader = &m_shaderInternal;
	m_scaleOffsetId = m_shader->getVariableId("ScaleOffset");
	m_texSizeId = m_shader->getVariableId("TexSize");
}
