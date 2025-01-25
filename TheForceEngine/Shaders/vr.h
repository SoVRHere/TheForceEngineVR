#if defined(OPT_VR_MULTIVIEW)
#if defined(VERTEX_SHADER)
layout(num_views = 2) in;
#endif
#endif

#if defined(OPT_VR) && defined(VERTEX_SHADER)
vec3 ProjectTo3D(vec2 screenCoord, vec2 screenDim, float distance, vec3 frustum[8])
{
	float ratioX = screenCoord.x / screenDim.x;
	float ratioY = screenCoord.y / screenDim.y;

	vec3 frustum4 = vec3(frustum[4].x, frustum[4].y, -distance);
	vec3 frustum7 = vec3(frustum[7].x, frustum[7].y, -distance);
	vec3 frustum6 = vec3(frustum[6].x, frustum[6].y, -distance);
	return frustum4 + ratioX * (frustum7 - frustum6) + ratioY * (frustum6 - frustum4);
}
#endif

#if defined(FRAGMENT_SHADER)
#if defined(OPT_VR_MULTIVIEW)
#define MV_SAMPLE_UV(uv) vec3((uv), gl_ViewID_OVR)
#define MV_SAMPLER2D sampler2DArray
#else
#define MV_SAMPLE_UV(uv) (uv)
#define MV_SAMPLER2D sampler2D
#endif
#endif
