#include "Shaders/vr.h"

//precision mediump float; // ES

#ifdef OPT_VR
uniform vec2 ScreenSize;
uniform vec3 Frustum[8];
uniform mat3 HmdView;
uniform vec4 Shift;
uniform int LockToCamera;
#ifdef OPT_VR_MULTIVIEW
uniform mat4 CameraProj_[2];
#define CameraProj CameraProj_[gl_ViewID_OVR]
#else
uniform mat4 CameraProj;
#endif
#else
uniform mat4 CameraProj;
#endif

in vec2 vtx_pos;
in vec2 vtx_uv;
in vec4 vtx_color;

out vec2 Frag_UV;
out vec4 Frag_Color;
#ifdef OPT_VR
out vec2 Frag_ScreenCoord;
#endif

void main()
{
    Frag_UV = vtx_uv;
    Frag_Color = vtx_color;

#ifdef OPT_VR
	Frag_ScreenCoord = vtx_pos;
	vec3 pos = ProjectTo3D(vec2(vtx_pos.x, ScreenSize.y - vtx_pos.y), ScreenSize, Shift.w, Frustum) + Shift.xyz;
	if (LockToCamera == 0)
		pos *= transpose(HmdView); // not lock to camera

	gl_Position = vec4(pos, 1.0) * CameraProj;
#else
    gl_Position = vec4(vtx_pos, 0.0, 1.0) * CameraProj;
#endif
}
