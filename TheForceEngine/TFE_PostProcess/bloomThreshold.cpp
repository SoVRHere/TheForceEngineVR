#include "bloomThreshold.h"
#include "postprocess.h"
#include <TFE_RenderBackend/renderBackend.h>
#include <TFE_System/profiler.h>
#include <TFE_Settings/settings.h>

bool BloomThreshold::init()
{
	m_bloomIntensityId = -1;
	m_texSizeId = -1;
	return buildShaders();
}

void BloomThreshold::destroy()
{
	m_shaderInternal.destroy();
}

bool BloomThreshold::buildShaders()
{
	ShaderDefine defines[1];
	defines[0].name = "OPT_OPTIMIZED";
	defines[0].value = "1";

	// Base shader.
	if (!m_shaderInternal.load("Shaders/bloom.vert", "Shaders/bloomThreshold.frag", m_optimized ? 1 : 0, defines, SHADER_VER_STD))
	{
		return false;
	}
	m_shaderInternal.bindTextureNameToSlot("ColorBuffer", 0);
	m_shaderInternal.bindTextureNameToSlot("MaterialBuffer", 1);
	m_bloomIntensityId = m_shaderInternal.getVariableId("bloomIntensity");
	setupShader();
	return true;
}

void BloomThreshold::setEffectState()
{
	TFE_RenderState::setStateEnable(false, STATE_CULLING | STATE_BLEND | STATE_DEPTH_TEST);
	if (m_shader)
	{
		f32 bloomIntensity = TFE_Settings::getGraphicsSettings()->bloomStrength * 8.0f;
		m_shader->setVariable(m_bloomIntensityId, SVT_SCALAR, &bloomIntensity);

		if (inputCount == 2)
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
}

void BloomThreshold::setupShader()
{
	m_shader = &m_shaderInternal;
	m_scaleOffsetId = m_shader->getVariableId("ScaleOffset");
	m_texSizeId = m_shader->getVariableId("TexSize");
}
