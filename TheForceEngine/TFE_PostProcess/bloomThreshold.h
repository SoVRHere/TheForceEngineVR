#pragma once
//////////////////////////////////////////////////////////////////////
// GPU Based blit.
//////////////////////////////////////////////////////////////////////

#include <TFE_System/types.h>
#include "postprocesseffect.h"

class BloomThreshold : public PostProcessEffect
{
public:
	BloomThreshold(bool optimized) : m_optimized(optimized) {}

	// Load shaders, setup geometry.
	bool init() override;
	// Free GPU assets.
	void destroy() override;

	// Execute the post process.
	void setEffectState() override;

private:
	bool m_optimized{ false };

	Shader m_shaderInternal;
	s32 m_bloomIntensityId;
	s32 m_texSizeId;

	void setupShader();
	bool buildShaders();
};
