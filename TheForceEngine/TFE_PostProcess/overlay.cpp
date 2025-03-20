#include "overlay.h"
#include "postprocess.h"
#include <TFE_RenderBackend/renderBackend.h>
#include <TFE_System/profiler.h>
#include <TFE_Settings/settings.h>

bool Overlay::init()
{
	return buildShaders();
}

void Overlay::destroy()
{
	m_overlayShader.destroy();
}

bool Overlay::buildShaders()
{
	// Base shader.
	m_overlayShader.load("Shaders/overlay.vert", "Shaders/overlay.frag");
	m_overlayShader.bindTextureNameToSlot("Image", 0);
	m_scaleOffsetId = m_overlayShader.getVariableId("ScaleOffset");
	m_tintId = m_overlayShader.getVariableId("Tint");
	m_uvOffsetSizeId = m_overlayShader.getVariableId("UvOffsetSize");

	if (TFE_Settings::getTempSettings()->vr)
	{
		m_cameraProjId = m_overlayShader.getVariableId("CameraProj");
		m_screenSizeId = m_overlayShader.getVariableId("ScreenSize");
		m_frustumId = m_overlayShader.getVariableId("Frustum");
		m_HmdViewId = m_overlayShader.getVariableId("HmdView");
		m_ShiftId = m_overlayShader.getVariableId("Shift");
		m_FovId = m_overlayShader.getVariableId("Fov");
		m_scaleId = m_overlayShader.getVariableId("Scale");
	}

	m_shader = &m_overlayShader;
	return true;
}

void Overlay::setEffectState()
{
	TFE_RenderState::setStateEnable(false, STATE_CULLING | STATE_DEPTH_TEST);
	TFE_RenderState::setStateEnable(true, STATE_BLEND);
	TFE_RenderState::setBlendMode(BLEND_ONE, BLEND_ONE_MINUS_SRC_ALPHA);
}
