#include "Shaders/vr.h"

#ifdef OPT_VR
uniform vec2 ScreenSize;
uniform vec3 Frustum[8];
uniform mat3 HmdView;
uniform vec4 Shift;
//uniform vec2 Fov;
//uniform float Scale;
#ifdef OPT_VR_MULTIVIEW
uniform mat4 CameraProj_[2];
#define CameraProj CameraProj_[gl_ViewID_OVR]
#else
uniform mat4 CameraProj;
#endif
#endif

uniform vec4 ScaleOffset;
uniform vec4 UvOffsetSize;
in vec2 vtx_pos;
in vec2 vtx_uv;

out vec2 Frag_UV;

void main()
{
    Frag_UV = vtx_uv * UvOffsetSize.xy + UvOffsetSize.zw;
#ifdef OPT_VR
	//float distanceToMpp = 2.0 * tan(Fov.y * 0.5) / ScreenSize.y;
	//float scale = 1.0 / (projDistance * distanceToMpp);
	vec2 pp = (vtx_pos.xy * ScaleOffset.xy + ScaleOffset.zw);// * (scale * Scale);
	pp = (pp + vec2(1.0, 1.0)) / vec2(2.0, 2.0);
	pp *= vec2(ScreenSize.x, ScreenSize.y);
	vec3 pos = ProjectTo3D(pp, ScreenSize, Shift.w, Frustum) + Shift.xyz;
	pos *= transpose(HmdView); // not lock to camera
	gl_Position = vec4(pos, 1.0) * CameraProj;
#else
    gl_Position = vec4(vtx_pos.xy * ScaleOffset.xy + ScaleOffset.zw, 0, 1);
#endif
}
