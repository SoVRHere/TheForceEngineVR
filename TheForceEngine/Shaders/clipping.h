#if defined(VERTEX_SHADER)
#ifdef GL_EXT_clip_cull_distance
#define Frag_ClipDistance gl_ClipDistance
#else
out float Frag_ClipDistance[8];
#endif
#elif defined(FRAGMENT_SHADER)
#ifdef GL_EXT_clip_cull_distance
void Clip() {}
#else
in float Frag_ClipDistance[8];
void Clip()
{
	for (int i = 0; i < 8; i++)
	{
		if (Frag_ClipDistance[i] < 0.0)
		{
			discard;
		}
	}
}
#endif
#endif
