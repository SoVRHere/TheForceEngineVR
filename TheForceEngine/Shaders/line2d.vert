#include "Shaders/vr.h"

#ifdef OPT_VR
uniform vec2 ScreenSize;
uniform vec3 Frustum[8];
uniform mat3 HmdView;
uniform vec4 Shift;
uniform int LockToCamera;
uniform float WidthMultiplier;
#ifdef OPT_VR_MULTIVIEW
uniform mat4 CameraProj_[2];
#define CameraProj CameraProj_[gl_ViewID_OVR]
#else
uniform mat4 CameraProj;
#endif
#endif

uniform vec4 ScaleOffset;

in vec4 vtx_pos;
in vec4 vtx_uv;
in vec4 vtx_uv1;
in vec4 vtx_color;
#ifdef OPT_CURVE
	flat out vec4 Frag_ControlAB;
	flat out vec2 Frag_ControlC;
	flat out vec4 Frag_Offsets;
#else
	out float Frag_Width;
	out vec4 Frag_UV;
#endif
out vec2 Frag_Pos;
out vec4 Frag_Color;

void main()
{
#ifdef OPT_VR
#ifdef OPT_CURVE
	Frag_ControlAB = vtx_uv;
	Frag_ControlC = vtx_pos.zw;
	Frag_Offsets = vtx_uv1;
#else  // Normal Line
    Frag_Width = vtx_pos.z;
    Frag_Width *= WidthMultiplier;
    Frag_UV = vtx_uv;
#endif
	Frag_Pos = vtx_pos.xy;
	Frag_Color = vtx_color;
	vec3 pos = ProjectTo3D(vtx_pos.xy, ScreenSize, Shift.w, Frustum) + Shift.xyz;
	if (LockToCamera == 0) 
		pos *= transpose(HmdView); // not lock to camera
	gl_Position = vec4(pos, 1.0) * CameraProj;
#else
    vec2 pos = vtx_pos.xy * ScaleOffset.xy + ScaleOffset.zw;
#ifdef OPT_CURVE
	Frag_ControlAB = vtx_uv;
	Frag_ControlC = vtx_pos.zw;
	Frag_Offsets = vtx_uv1;
#else  // Normal Line
    Frag_Width = vtx_pos.z;
    Frag_UV = vtx_uv;
#endif
	Frag_Pos = vtx_pos.xy;
	Frag_Color = vtx_color;
    gl_Position = vec4(pos.xy, 0, 1);
#endif
}

