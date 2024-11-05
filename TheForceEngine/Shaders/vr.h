#ifdef OPT_VR_MULTIVIEW
#extension GL_OVR_multiview2 : enable
#ifdef VERTEX_SHADER
layout(num_views = 2) in;
#endif
#endif

#if defined(OPT_VR) && defined(VERTEX_SHADER)
vec3 ProjectTo3D(vec2 screenCoord, vec2 screenDim, float distance, vec3 frustum[8])
{
	float ratioX = screenCoord.x / screenDim.x;
	float ratioY = screenCoord.y / screenDim.y;

	vec3 p0 = frustum[0/*fp_NearTopLeft*/] + ratioX * (frustum[3] - frustum[2]) + ratioY * (frustum[2] - frustum[0]);
	vec3 p1 = frustum[4/*fp_FarTopLeft*/] + ratioX * (frustum[7] - frustum[6]) + ratioY * (frustum[6] - frustum[4]);
	vec3 v = p1 - p0; // vector from near plane to far plane from screen point

	vec3 at = vec3(0.0, 0.0, -1.0);//normalize(-eye[2].xyz);

	// ax + by + cz + d = 0
	// n = [a, b, c] = normal to plane
	// p = [x, y, z] = point on plane
	// d = -(ax + by + cz) = -dot(n, p)
	vec3 n = at;
	vec3 p = vec3(0.0, 0.0, 0.0)/*eye[3].xyz*/ + at * distance;
	float d = -dot(n, p);

	float t = -(dot(n, p0) + d) / dot(n, v); // line vs plane intersection
	//vec4 cc = eyeInv * vec4(p0 + h_userScale * t * v, 1.0);
	vec3 pos = p0 + t * v;
	return pos;
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
