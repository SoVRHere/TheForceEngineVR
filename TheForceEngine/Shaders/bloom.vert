#include "Shaders/vr.h"

#if !defined(OPT_OPTIMIZED)

uniform vec4 ScaleOffset;
in vec2 vtx_pos;
in vec2 vtx_uv;

out vec2 Frag_UV;

void main()
{
    Frag_UV = vec2(vtx_uv.x, 1.0 - vtx_uv.y);
    gl_Position = vec4(vtx_pos.xy * ScaleOffset.xy + ScaleOffset.zw, 0, 1);
}

#else

uniform vec4 ScaleOffset;
uniform vec2 TexSize;

in vec2 vtx_pos;
in vec2 vtx_uv;

out vec4 Frag_UV0;
out vec4 Frag_UV1;
out vec4 Frag_UV2;
out vec4 Frag_UV3;
out vec4 Frag_UV4;
out vec4 Frag_UV5;
out vec4 Frag_UV6;

vec2 getUV(vec2 uv, vec2 step, float x, float y)
{
	return uv + step * vec2(x, y);
}

void main()
{
    vec2 baseUV = vec2(vtx_uv.x, 1.0 - vtx_uv.y);
	vec2 step = vec2(1.0) / TexSize;

	Frag_UV0.xy = getUV(baseUV, step, -1.0,  1.0);
	Frag_UV0.zw = getUV(baseUV, step,  1.0,  1.0);
	Frag_UV1.xy = getUV(baseUV, step, -1.0, -1.0);
	Frag_UV1.zw = getUV(baseUV, step,  1.0, -1.0);

	// Low Weights (0.5 / 9.0)
	Frag_UV2.xy = getUV(baseUV, step, -2.0,  2.0);
	Frag_UV2.zw = getUV(baseUV, step,  0.0,  2.0);
	Frag_UV3.xy = getUV(baseUV, step,  2.0,  2.0);

	Frag_UV3.zw = getUV(baseUV, step, -2.0,  0.0);
	Frag_UV4.xy = getUV(baseUV, step,  0.0,  0.0);
	Frag_UV4.zw = getUV(baseUV, step,  2.0,  0.0);

	Frag_UV5.xy = getUV(baseUV, step, -2.0, -2.0);
	Frag_UV5.zw = getUV(baseUV, step,  0.0, -2.0);
	Frag_UV6.xy = getUV(baseUV, step,  2.0, -2.0);

    gl_Position = vec4(vtx_pos.xy * ScaleOffset.xy + ScaleOffset.zw, 0, 1);
}

#endif
