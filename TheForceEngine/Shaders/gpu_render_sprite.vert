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

uniform isamplerBuffer TextureTable;
uniform samplerBuffer DrawListPosXZ_Texture;
uniform samplerBuffer DrawListPosYU_Texture;
uniform isamplerBuffer DrawListTexId_Texture;

uniform samplerBuffer DrawListPlanes;

// in int gl_VertexID;
out vec2 Frag_Uv; // base uv coordinates (0 - 1)
out vec3 Frag_Pos;     // camera relative position for lighting.
flat out int Frag_TextureId;
flat out vec4 Texture_Data; // not much here yet.

void unpackPortalInfo(uint portalInfo, out uint portalOffset, out uint portalCount)
{
	portalCount  = (portalInfo >> 16u) & 15u;
	portalOffset = portalInfo & 65535u;
}

void main()
{
	// We do our own vertex fetching and geometry expansion, so calculate the relevent values from the vertex ID.
	int spriteIndex = gl_VertexID / 4;
	int vertexId  = gl_VertexID & 3;
	
	vec4 posTextureXZ = texelFetch(DrawListPosXZ_Texture, spriteIndex);
	vec4 posTextureYU = texelFetch(DrawListPosYU_Texture, spriteIndex);
	uvec2 texPortalData = uvec2(texelFetch(DrawListTexId_Texture, spriteIndex).rg);
	uint tex_flags = texPortalData.x;
	Frag_TextureId = int(tex_flags & 32767u);

	float u = float(vertexId&1);
	float v = float(1-(vertexId/2));

	vec3 vtx_pos;
	vtx_pos.xz = mix(posTextureXZ.xy, posTextureXZ.zw, u);
	vtx_pos.y  = mix(posTextureYU.x, posTextureYU.y, v);

	ivec2 sh = texelFetch(TextureTable, Frag_TextureId).yw;
	float scaleFactor = 1.0 / float(sh.x >> 12);

	vec2 vtx_uv;
	vtx_uv.x = mix(posTextureYU.z, posTextureYU.w, u);
	vtx_uv.y = v * float(sh.y) * scaleFactor;

	vec4 texture_data = vec4(0.0);
	texture_data.y = float((tex_flags >> 16u) & 31u);

	// Calculate vertical clipping.
	uint portalInfo = texPortalData.y;
	uint portalOffset, portalCount;
	unpackPortalInfo(portalInfo, portalOffset, portalCount);

	// Clipping.
	gl_ClipDistance[7] = 1.0; // must be sized by the shader either by redeclaring it with an explicit size, or by indexing it with only integral constant expressions
	for (int i = 0; i < int(portalCount) && i < 8; i++)
	{
		vec4 plane = texelFetch(DrawListPlanes, int(portalOffset) + i);
		gl_ClipDistance[i] = dot(vec4(vtx_pos.xyz, 1.0), plane);
	}
	for (int i = int(portalCount); i < 8; i++)
	{
		gl_ClipDistance[i] = 1.0;
	}

	// Relative position
	Frag_Pos = vtx_pos - CameraPos;
	
	// Transform from world to view space.
    vec3 vpos = (vtx_pos - CameraPos) * CameraView;
	gl_Position = vec4(vpos, 1.0) * CameraProj;
	
	// Write out the per-vertex uv and color.
	Frag_Uv = vtx_uv;
	Texture_Data = texture_data;
}
