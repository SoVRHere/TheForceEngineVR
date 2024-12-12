#include "Shaders/vr.h"

//precision mediump float; // ES

#ifdef OPT_VR
uniform vec2 ScreenSize;
uniform vec3 Frustum[8];
uniform mat4 Eye[2];
uniform mat3 HmdView;
uniform vec4 Shift;
uniform int LockToCamera;
uniform mat4 Controller[2];
uniform float CtrlGripTrigger[2];
uniform float CtrlIndexTrigger[2];
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
	float leftTrigger = CtrlIndexTrigger[0];
	vec3 shift = vec3(Shift.x, Shift.y, (1.0 - leftTrigger) * Shift.z);
	vec3 pos = ProjectTo3D(vec2(vtx_pos.x, ScreenSize.y - vtx_pos.y), ScreenSize, Shift.w, Frustum) + shift;
	mat4 ctrl = mat4(Controller[0]);
	//ctrl[3] = vec4(0, 0, 0, 1);
	vec4 cc = ctrl * vec4(pos, 1.0);
	if (LockToCamera == 0)
	{
		mat4 hmdView4 = mat4(HmdView);
		hmdView4[3] = vec4(0, 0, 0, 1);
		cc *= transpose(hmdView4); // not lock to camera
	}

	gl_Position = cc * CameraProj;
#else
    gl_Position = vec4(vtx_pos, 0.0, 1.0) * CameraProj;
#endif
}
