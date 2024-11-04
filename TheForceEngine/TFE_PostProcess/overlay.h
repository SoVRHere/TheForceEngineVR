#pragma once
//////////////////////////////////////////////////////////////////////
// GPU Based blit.
//////////////////////////////////////////////////////////////////////

#include <TFE_System/types.h>
#include "postprocesseffect.h"

class Overlay : public PostProcessEffect
{
public:
	// Load shaders, setup geometry.
	bool init() override;
	// Free GPU assets.
	void destroy() override;

	// Execute the post process.
	void setEffectState() override;

public:
	s32 m_tintId;
	s32 m_uvOffsetSizeId;

	s32 m_cameraProjId = -1;
	s32 m_screenSizeId = -1;
	s32 m_frustumId = -1;
	s32 m_HmdViewId = -1;
	s32 m_ShiftId = -1;
	s32 m_FovId = -1;
	s32 m_scaleId = -1;

private:
	Shader m_overlayShader;
	bool buildShaders();
};
