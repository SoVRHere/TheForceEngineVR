#include "Shaders/vr.h"

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
#endif

uniform vec4 ScaleOffset;
in vec2 vtx_pos;
in vec2 vtx_uv;

out vec2 Frag_UV;

void main()
{
    Frag_UV = vtx_uv;
#if defined(OPT_VR) && defined(ENABLE_GPU_COLOR_CONVERSION)
	// PDA & videos are always rendered 4:3
	// so we have to adjust it to ScreenSize.x:ScreenSize.y
	float aspect = ScreenSize.x / ScreenSize.y;
	vec2 pp = vtx_pos;
	//if (aspect < 4.0/3.0)
	{
		float relHeight = (aspect / 4.0) * 3.0;
		float offsetY = relHeight / 2.0;
		pp.y += offsetY;
		pp *= vec2(ScreenSize.x, ScreenSize.y * relHeight);
	}
	//else
	{
		// TODO:
	}

	vec3 pos = ProjectTo3D(pp, ScreenSize, Shift.w, Frustum) + Shift.xyz;
	if (LockToCamera == 0) 
		pos *= transpose(HmdView); // not lock to camera
	gl_Position = vec4(pos, 1.0) * CameraProj;
#else
    gl_Position = vec4(vtx_pos.xy * ScaleOffset.xy + ScaleOffset.zw, 0, 1);
#endif
}
