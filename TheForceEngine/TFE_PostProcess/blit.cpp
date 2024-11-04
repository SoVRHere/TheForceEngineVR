#include "blit.h"
#include "postprocess.h"
#include <TFE_RenderBackend/renderBackend.h>
#include <TFE_System/profiler.h>
#include <TFE_Settings/settings.h>
#include <TFE_System/math.h>
#include <TFE_Vr/vr.h>

namespace TFE_RenderBackend
{
	extern Mat4  s_cameraProjVR[2];
	extern Mat4  s_cameraProjVR_YDown[2];
	extern Mat3  s_cameraMtxVR[2];
	extern Mat3  s_cameraMtxVR_YDown[2];
	extern Vec3f s_cameraPosVR[2];
	extern Vec3f s_cameraPosVR_YDown[2];
}

bool Blit::init()
{
	bool res = buildShaders();
	enableFeatures(0);

	m_colorCorrectParam = { 1.0f, 1.0f, 1.0f, 1.0f };
	m_colorCorrectVarId = -1;

	m_cameraProjId = -1;
	m_screenSizeId = -1;
	m_frustumId = -1;
	m_HmdViewId = -1;
	m_ShiftId = -1;
	m_LockToCameraId = -1;

	return res;
}

void Blit::destroy()
{
	for (u32 i = 0; i < BLIT_FEATURE_COMBO_COUNT; i++)
	{
		m_featureShaders[i].destroy();
	}
}
u32 addVRDefines(size_t index, ShaderDefine defines[])
{
	u32 added = 0;
	if (TFE_Settings::getTempSettings()->vr)
	{
		defines[index++] = { "OPT_VR", "1" };
		added++;

		if (TFE_Settings::getTempSettings()->vrMultiview)
		{
			defines[index++] = { "OPT_VR_MULTIVIEW", "1" };
			added++;
		}
	}
	return added;
}

bool Blit::buildShaders()
{
	ShaderDefine defines[BLIT_FEATURE_COUNT + 2/*VR*/];

	// Base shader.
	u32 added = addVRDefines(0, defines);
	m_featureShaders[0].load("Shaders/blit.vert", "Shaders/blit.frag", added, defines, SHADER_VER_STD);
	m_featureShaders[0].bindTextureNameToSlot("VirtualDisplay", 0);

	// BLIT_GPU_COLOR_CONVERSION feature
	defines[0] = { "ENABLE_GPU_COLOR_CONVERSION", "1" };
	added = addVRDefines(1, defines);
	m_featureShaders[1].load("Shaders/blit.vert", "Shaders/blit.frag", 1 + added, defines, SHADER_VER_STD);
	m_featureShaders[1].bindTextureNameToSlot("VirtualDisplay", 0);
	m_featureShaders[1].bindTextureNameToSlot("Palette", 1);

	// BLIT_GPU_COLOR_CORRECTION feature
	defines[0] = { "ENABLE_GPU_COLOR_CORRECTION", "1" };
	added = addVRDefines(1, defines);
	m_featureShaders[2].load("Shaders/blit.vert", "Shaders/blit.frag", 1 + added, defines, SHADER_VER_STD);
	m_featureShaders[2].bindTextureNameToSlot("VirtualDisplay", 0);

	// BLIT_GPU_COLOR_CONVERSION + BLIT_GPU_COLOR_CORRECTION features
	defines[0] = { "ENABLE_GPU_COLOR_CONVERSION", "1" };
	defines[1] = { "ENABLE_GPU_COLOR_CORRECTION", "1" };
	added = addVRDefines(2, defines);
	m_featureShaders[3].load("Shaders/blit.vert", "Shaders/blit.frag", 2 + added, defines, SHADER_VER_STD);
	m_featureShaders[3].bindTextureNameToSlot("VirtualDisplay", 0);
	m_featureShaders[3].bindTextureNameToSlot("Palette", 1);


	// BLIT_BLOOM feature
	defines[0] = { "ENABLE_BLOOM", "1" };
	added = addVRDefines(1, defines);
	m_featureShaders[4].load("Shaders/blit.vert", "Shaders/blit.frag", 1 + added, defines, SHADER_VER_STD);
	m_featureShaders[4].bindTextureNameToSlot("VirtualDisplay", 0);
	m_featureShaders[4].bindTextureNameToSlot("Bloom", 1);

	// BLIT_BLOOM + BLIT_GPU_COLOR_CORRECTION feature
	defines[0] = { "ENABLE_BLOOM", "1" };
	defines[1] = { "ENABLE_GPU_COLOR_CORRECTION", "1" };
	added = addVRDefines(2, defines);
	m_featureShaders[5].load("Shaders/blit.vert", "Shaders/blit.frag", 2 + added, defines, SHADER_VER_STD);
	m_featureShaders[5].bindTextureNameToSlot("VirtualDisplay", 0);
	m_featureShaders[5].bindTextureNameToSlot("Bloom", 1);

	return true;
}

void Blit::setEffectState()
{
	TFE_RenderState::setStateEnable(false, STATE_CULLING | STATE_BLEND | STATE_DEPTH_TEST);

	if (m_features & BLIT_GPU_COLOR_CORRECTION)
	{
		m_shader->setVariable(m_colorCorrectVarId, SVT_VEC4, m_colorCorrectParam.m);
	}

	if (TFE_Settings::getTempSettings()->vr)
	{
		const std::array<Vec3f, 8>& frustum = vr::GetUnitedFrustum();
		const Mat3 hmdMtx = TFE_Math::getMatrix3(vr::GetEyePose(vr::Side::Left).mTransformation);

		const Vec2ui& targetSize = vr::GetRenderTargetSize();
		const TFE_Settings_Vr* vrSettings = TFE_Settings::getVrSettings();

		m_shader->setVariableArray(m_cameraProjId, SVT_MAT4x4, TFE_RenderBackend::s_cameraProjVR[0].data, 2);
		m_shader->setVariable(m_screenSizeId, SVT_VEC2, Vec2f{ f32(targetSize.x), f32(targetSize.y) }.m);
		m_shader->setVariableArray(m_frustumId, SVT_VEC3, frustum.data()->m, (u32)frustum.size());
		m_shader->setVariable(m_HmdViewId, SVT_MAT3x3, hmdMtx.data);
		const Vec3f& shift = vrSettings->pdaToVr.shift;
		m_shader->setVariable(m_ShiftId, SVT_VEC4, Vec4f{ shift.x, shift.y, shift.z, vrSettings->pdaToVr.distance }.m);
		s32 lock = vrSettings->pdaToVr.lockToCamera ? 1 : 0;
		m_shader->setVariable(m_LockToCameraId, SVT_ISCALAR, &lock);
	}
}

void Blit::setupShader()
{
	m_shader = &m_featureShaders[0];
	if ((m_features & BLIT_GPU_COLOR_CONVERSION) && (m_features & BLIT_GPU_COLOR_CORRECTION))
	{
		m_shader = &m_featureShaders[3];
	}
	else if (m_features & BLIT_GPU_COLOR_CONVERSION)
	{
		m_shader = &m_featureShaders[1];
	}
	else if ((m_features & BLIT_GPU_COLOR_CORRECTION) && (m_features & BLIT_BLOOM))
	{
		m_shader = &m_featureShaders[5];
	}
	else if (m_features & BLIT_GPU_COLOR_CORRECTION)
	{
		m_shader = &m_featureShaders[2];
	}
	else if (m_features & BLIT_BLOOM)
	{
		m_shader = &m_featureShaders[4];
	}
	m_scaleOffsetId = m_shader->getVariableId("ScaleOffset");

	if (m_features & BLIT_GPU_COLOR_CORRECTION)
	{
		m_colorCorrectVarId = m_shader->getVariableId("ColorCorrectParam");
	}

	if (TFE_Settings::getTempSettings()->vr)
	{
		m_cameraProjId = m_shader->getVariableId("CameraProj");
		m_screenSizeId = m_shader->getVariableId("ScreenSize");
		m_frustumId = m_shader->getVariableId("Frustum");
		m_HmdViewId = m_shader->getVariableId("HmdView");
		m_ShiftId = m_shader->getVariableId("Shift");
		m_LockToCameraId = m_shader->getVariableId("LockToCamera");
	}
}

void Blit::enableFeatures(u32 features)
{
	m_features |= features;
	setupShader();
}

void Blit::disableFeatures(u32 features)
{
	m_features &= ~features;
	setupShader();
}

bool Blit::featureEnabled(u32 feature) const
{
	return (m_features & feature) != 0;
}

void Blit::setColorCorrectionParameters(const ColorCorrection* parameters)
{
	m_colorCorrectParam.x = parameters->brightness;
	m_colorCorrectParam.y = parameters->contrast;
	m_colorCorrectParam.z = parameters->saturation;
	m_colorCorrectParam.w = parameters->gamma;
}
