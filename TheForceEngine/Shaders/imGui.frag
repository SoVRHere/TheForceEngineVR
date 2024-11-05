#include "Shaders/vr.h"

uniform sampler2D Image;
#ifdef OPT_VR
uniform vec2 MousePos;
uniform float DotSize;
uniform vec4 DotColor;
uniform vec4 ClipRect;
#endif

in vec2 Frag_UV;
in vec4 Frag_Color;
#ifdef OPT_VR
in vec2 Frag_ScreenCoord;
#endif

out vec4 Out_Color;

void main()
{
	Out_Color = Frag_Color * texture(Image, Frag_UV.st);
#ifdef OPT_VR
	if (Frag_ScreenCoord.x < ClipRect.x || Frag_ScreenCoord.x > ClipRect.z || Frag_ScreenCoord.y < ClipRect.y || Frag_ScreenCoord.y > ClipRect.w)
	{
		Out_Color = vec4(0.0);
	}
	else
	if (length(MousePos - Frag_ScreenCoord) < 0.5 * DotSize)
	{
		Out_Color = vec4(DotColor.xyz * DotColor.w + Out_Color.xyz * (1.0 - DotColor.w), Out_Color.w);
	}
#endif
}
