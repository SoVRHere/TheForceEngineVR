#include "rcommon.h"
#include "redgePair.h"

struct SecObject;

namespace TFE_Jedi
{
	// Resolution
	s32 s_width = 320;
	s32 s_height = 200;
	f32 s_halfWidth;
	f32 s_halfHeight;
	f32 s_halfHeightBase;
	s32 s_screenYMid;
	s32 s_screenYMidBase;
	s32 s_minScreenY;
	s32 s_maxScreenY;
	s32 s_screenXMid;

	// Projection
	f32 s_focalLength;
	f32 s_focalLenAspect;
	f32 s_aspectScaleX;
	f32 s_aspectScaleY;
	f32 s_eyeHeight;
	f32* s_depth1d_all = nullptr;
	f32* s_depth1d = nullptr;

	// Camera
	f32 s_cameraPosX;
	f32 s_cameraPosY;
	f32 s_cameraPosZ;
	f32 s_xCameraTrans;
	f32 s_zCameraTrans;
	f32 s_cosYaw;
	f32 s_sinYaw;
	f32 s_negSinYaw;
	f32 s_cameraYaw;
	f32 s_cameraPitch;
	f32 s_yPlaneTop;
	f32 s_yPlaneBot;
	f32 s_skyYawOffset;
	f32 s_skyPitchOffset;
	f32 s_nearPlaneHalfLen = 1.0f;
	f32* s_skyTable;
	f32 s_cameraMtx[9] = { 0 };

	// Window
	s32 s_minScreenX;
	s32 s_maxScreenX;
	s32 s_windowMinX;
	s32 s_windowMaxX;
	s32 s_windowMinY;
	s32 s_windowMaxY;
	s32 s_windowMaxCeil;
	s32 s_windowMinFloor;
	f32 s_windowMinZ;

	// Display
	u8* s_display;

	// Render
	RSector* s_prevSector;
	s32 s_sectorIndex;
	s32 s_maxAdjoinIndex;
	s32 s_adjoinIndex;
	s32 s_maxAdjoinDepth;
	s32 s_windowX0;
	s32 s_windowX1;

	// Column Heights
	s32* s_columnTop = nullptr;
	s32* s_columnBot = nullptr;
	s32* s_windowTop_all = nullptr;
	s32* s_windowBot_all = nullptr;
	s32* s_windowTop = nullptr;
	s32* s_windowBot = nullptr;
	s32* s_windowTopPrev = nullptr;
	s32* s_windowBotPrev = nullptr;

	s32* s_objWindowTop = nullptr;
	s32* s_objWindowBot = nullptr;

	// Segment list.
	s32 s_nextWall;
	s32 s_curWallSeg;
	s32 s_adjoinSegCount;
	s32 s_adjoinDepth;
	s32 s_drawFrame = 0;

	// Flats
	s32 s_flatCount;
	s32 s_wallMaxCeilY;
	s32 s_wallMinFloorY;

	EdgePair* s_flatEdge;
	EdgePair  s_flatEdgeList[MAX_SEG];
	EdgePair* s_adjoinEdge;
	EdgePair  s_adjoinEdgeList[MAX_ADJOIN_SEG];

	// Lighting
	const u8* s_colorMap = nullptr;
	const u8* s_lightSourceRamp = nullptr;
	s32 s_flatAmbient = 0;
	s32 s_sectorAmbient;
	s32 s_scaledAmbient;
	s32 s_cameraLightSource;
	JBool s_enableFlatShading;
	s32 s_worldAmbient;
	s32 s_sectorAmbientFraction;
	s32 s_lightCount = 3;
	JBool s_flatLighting = JFALSE;

	// Debug
	s32 s_maxWallCount;
	s32 s_maxDepthCount;

	s32 s_drawnSpriteCount;
	SecObject* s_drawnSprites[MAX_DRAWN_SPRITE_STORE];
}
