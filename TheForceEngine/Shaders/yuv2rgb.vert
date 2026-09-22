#include "Shaders/vr.h"

uniform vec4 ScaleOffset;
// pic_height / frame_height: scales V so we only sample the valid
// picture rows and never touch the macroblock-alignment padding at
// the bottom of the texture (e.g. 1080/1088 for a 1920x1088 OGV).
uniform float UVScale;
in vec2 vtx_pos;
in vec2 vtx_uv;

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
#uniform mat4 CameraProj;
#endif
//#else
//uniform mat4 CameraProj;
#endif


out vec2 Frag_UV;

void main()
{
#ifdef OPT_VR
	Frag_UV = vec2(vtx_uv.x, vtx_uv.y * UVScale);
	vec2 pos2d = 0.5 * ScreenSize * (vtx_pos.xy * ScaleOffset.xy + ScaleOffset.zw + 1.0);
	vec3 pos = ProjectTo3D(vec2(pos2d.x , ScreenSize.y - pos2d.y), ScreenSize, Shift.w, Frustum) + Shift.xyz;
	if (LockToCamera == 0)
		pos *= transpose(HmdView); // not locked to camera
	gl_Position = vec4(pos, 1.0) * CameraProj;
#else
	Frag_UV = vec2(vtx_uv.x, (1.0 - vtx_uv.y) * UVScale);
	gl_Position = vec4(vtx_pos.xy * ScaleOffset.xy + ScaleOffset.zw, 0, 1);
#endif
}
