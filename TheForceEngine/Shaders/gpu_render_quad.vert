#include "Shaders/vr.h"

uniform vec4 ScaleOffset;

#ifdef OPT_VR
uniform vec2 ScreenSize;
uniform vec3 Frustum[8];
uniform mat3 HmdView;
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
#endif

in vec4 vtx_pos;
in vec4 vtx_uv;
in int vtx_uv1;
in vec4 vtx_color;

out vec2 Frag_Uv;		// base uv coordinates (0 - 1)
flat out vec4 Frag_TextureId_Color;

void main()
{
	Frag_Uv = vtx_pos.zw;
	Frag_TextureId_Color = vtx_color;

#ifdef OPT_VR
	vec3 pos = ProjectTo3D(vtx_pos.xy, ScreenSize, vtx_uv.w, Frustum) + vtx_uv.xyz;
	if (vtx_uv1 == 0) 
		pos *= transpose(HmdView); // not lock to camera
	gl_Position = vec4(pos, 1.0) * CameraProj;
#else
	vec2 pos = vtx_pos.xy*ScaleOffset.xy + ScaleOffset.zw;
	gl_Position = vec4(pos.xy, 0, 1);
#endif
}
