#include "Shaders/vr.h"

#ifdef OPT_VR_MULTIVIEW
uniform vec3 CameraPos_[2];
uniform mat3 CameraView_[2];
uniform mat4 CameraProj_[2];
#define CameraPos CameraPos_[gl_ViewID_OVR]
#define CameraView CameraView_[gl_ViewID_OVR]
#define CameraProj CameraProj_[gl_ViewID_OVR]
#else
uniform vec3 CameraPos;
uniform mat3 CameraView;
uniform mat4 CameraProj;
#endif
uniform vec3 CameraRight;

uniform mat3 ModelMtx;
uniform vec3 ModelPos;
uniform uvec2 PortalInfo;

uniform samplerBuffer DrawListPlanes;
#ifdef OPT_TRUE_COLOR
uniform sampler2D BasePalette;
#endif

// Vertex Data
in vec3 vtx_pos;
in vec2 vtx_uv;
in vec4 vtx_color;

void unpackPortalInfo(uint portalInfo, out uint portalOffset, out uint portalCount)
{
	portalCount  = (portalInfo >> 16u) & 15u;
	portalOffset = portalInfo & 65535u;
}

#ifdef OPT_TRUE_COLOR
flat out vec3 Frag_Color;
#else
flat out int Frag_Color;
#endif

void main()
{
	// Transform by the model matrix.
	vec3 worldPos = vtx_pos * ModelMtx + ModelPos;

	vec3 centerPos = (worldPos - CameraPos) * CameraView;
	float scale = abs(0.5/200.0 * centerPos.z);
	
	// Expand the quad, aligned to the right vector and world up.
	worldPos.xz -= vec2(scale) * CameraRight.xz;
	worldPos.xz += vec2(2.0*scale) * CameraRight.xz * vtx_uv.xx;
	worldPos.y  += scale - 2.0*scale * vtx_uv.y;

	// Clipping.
	uint portalOffset, portalCount;
	unpackPortalInfo(PortalInfo.x, portalOffset, portalCount);
	gl_ClipDistance[7] = 1.0; // must be sized by the shader either by redeclaring it with an explicit size, or by indexing it with only integral constant expressions
	for (int i = 0; i < int(portalCount) && i < 8; i++)
	{
		vec4 plane = texelFetch(DrawListPlanes, int(portalOffset) + i);
		gl_ClipDistance[i] = dot(vec4(worldPos.xyz, 1.0), plane);
	}
	for (int i = int(portalCount); i < 8; i++)
	{
		gl_ClipDistance[i] = 1.0;
	}

	// Transform from world to view space.
    vec3 vpos = (worldPos - CameraPos) * CameraView;
	gl_Position = vec4(vpos, 1.0) * CameraProj;
	
	// Write out the per-vertex uv and color.
#ifdef OPT_TRUE_COLOR
	int palIndex = int(vtx_color.x * 255.0 + 0.5);
	Frag_Color = texelFetch(BasePalette, ivec2(palIndex, 0), 0).rgb;
#else
	Frag_Color = int(vtx_color.x * 255.0 + 0.5);
#endif
}
