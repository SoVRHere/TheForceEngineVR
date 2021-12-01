#include <cstring>

#include "pda.h"
#include "menu.h"
#include "missionBriefing.h"
#include <TFE_DarkForces/Landru/lactorDelt.h>
#include <TFE_DarkForces/Landru/lactorAnim.h>
#include <TFE_DarkForces/Landru/lpalette.h>
#include <TFE_DarkForces/Landru/lcanvas.h>
#include <TFE_DarkForces/Landru/ldraw.h>
#include <TFE_DarkForces/automap.h>
#include <TFE_Archive/lfdArchive.h>
#include <TFE_DarkForces/agent.h>
#include <TFE_DarkForces/util.h>
#include <TFE_Archive/archive.h>
#include <TFE_Settings/settings.h>
#include <TFE_Input/input.h>
#include <TFE_RenderBackend/renderBackend.h>
#include <TFE_Jedi/Math/core_math.h>
#include <TFE_Jedi/Renderer/virtualFramebuffer.h>
#include <TFE_Jedi/Renderer/screenDraw.h>
#include <TFE_Jedi/Level/rtexture.h>
#include <TFE_System/system.h>

using namespace TFE_Jedi;

namespace TFE_DarkForces
{
	///////////////////////////////////////////
	// Constants
	///////////////////////////////////////////
	enum PdaMode
	{
		PDA_MODE_MAP = 0,
		PDA_MODE_WEAPONS,
		PDA_MODE_INV,
		PDA_MODE_GOALS,
		PDA_MODE_BRIEF,
		PDA_MODE_COUNT
	};

	enum PdaButton
	{
		PDA_BTN_MAP = 0,
		PDA_BTN_WEAPONS,
		PDA_BTN_INV,
		PDA_BTN_GOALS,
		PDA_BTN_BRIEF,
		PDA_BTN_EXIT,

		PDA_BTN_PANUP,
		PDA_BTN_PANRIGHT,
		PDA_BTN_PANDOWN,
		PDA_BTN_PANLEFT,
		PDA_BTN_ZOOMOUT,
		PDA_BTN_ZOOMIN,
		PDA_BTN_LAYERUP,
		PDA_BTN_LAYERDOWN,

		PDA_BTN_COUNT,
		PDA_RADIO_BTN_COUNT = PDA_BTN_BRIEF - PDA_BTN_MAP + 1
	};

	enum PdaFrames
	{
		PFRAME_BACK = 0,
		PFRAME_SECRET_LABEL = 30,
		PFRAME_SECRET_BOX = 31,
	};

	///////////////////////////////////////////
	// State
	///////////////////////////////////////////
	static JBool s_pdaOpen   = JFALSE;
	static JBool s_pdaLoaded = JFALSE;
	static LActor* s_briefing = nullptr;
	static LActor* s_pdaArt  = nullptr;
	static LActor* s_goals   = nullptr;
	static LActor* s_weapons = nullptr;
	static LActor* s_items   = nullptr;
	static LPalette* s_palette = nullptr;
	static LRect s_pdaRect   = { 12, 15, 168, 305 };
	static LRect s_viewBounds;
	static u8* s_framebuffer;

	static PdaMode s_pdaMode = PDA_MODE_MAP;
	static s32 s_simulatePressed = -1;
		
//#define	MISSION_BRIEF_LFD		"dfbrief.lfd"
//#define	WEAPONS_ANIM			"guns"	// guns.ani in DFBRIEF.LFD
//#define	ITEMS_ANIM				"items"
//#define	NUM_WEAPONS				9
//#define	SECRET_COLOR			19
//#define	SECRET_COLOR_SHDW		91

	///////////////////////////////////////////
	// API Implementation
	///////////////////////////////////////////
	void pda_start(const char* levelName)
	{
		if (!s_pdaLoaded)
		{
			if (!menu_openResourceArchive("dfbrief.lfd"))
			{
				return;
			}
			LRect rect;

			s_briefing = lactorDelt_load(levelName, &s_pdaRect, 0, 0, 0);
			s_goals    = lactorAnim_load(levelName, &rect, 0, 0, 0);
			s_weapons  = lactorAnim_load("guns",  &rect, 0, 0, 0);
			s_items    = lactorAnim_load("items", &rect, 0, 0, 0);
			menu_closeResourceArchive();

			lactor_setTime(s_goals,   -1, -1);
			lactor_setTime(s_weapons, -1, -1);
			lactor_setTime(s_items,   -1, -1);

			if (s_briefing)
			{
				lactor_setTime(s_briefing, -1, -1);
				lactorDelt_getFrame(s_briefing, &rect);

				s_briefingMaxY = (rect.bottom - rect.top) - (s_pdaRect.bottom - s_pdaRect.top) + BRIEF_VERT_MARGIN;
				// Round to a factor of the line scrolling value.
				s_briefingMaxY = s_briefingMaxY + BRIEF_LINE_SCROLL - (s_briefingMaxY % BRIEF_LINE_SCROLL);

				if (s_briefingMaxY < -BRIEF_VERT_MARGIN)
				{
					s_briefingMaxY = -BRIEF_VERT_MARGIN;
				}
				s_briefY = -BRIEF_VERT_MARGIN;
			}
			s_overlayRect = s_pdaRect;

			if (!menu_openResourceArchive("menu.lfd"))
			{
				return;
			}
			s_framebuffer = ldraw_getBitmap();
			lcanvas_getBounds(&s_viewBounds);

			s_pdaArt = lactorAnim_load("pda", &s_viewBounds, 0, 0, 0);
			s_palette = lpalette_load("menu");
			lactor_setTime(s_pdaArt, -1, -1);
			lpalette_setScreenPal(s_palette);

			menu_closeResourceArchive();

			s_pdaLoaded = JTRUE;
		}
		s_pdaOpen = JTRUE;
		s_buttonPressed = -1;
		s_simulatePressed = -1;
		s_buttonHover = JFALSE;
		automap_updateMapData(MAP_CENTER_PLAYER);
		automap_updateMapData(MAP_ENABLE_AUTOCENTER);
		automap_resetScale();

		TFE_Input::endFrame();
	}

	void pda_resetState()
	{
		s_pdaOpen   = JFALSE;
		s_pdaLoaded = JFALSE;
		s_pdaArt  = nullptr;
		s_goals   = nullptr;
		s_weapons = nullptr;
		s_items   = nullptr;
	}

	JBool pda_isOpen()
	{
		return s_pdaOpen;
	}

	void pda_handleButtons()
	{
		if (TFE_Input::mousePressed(MBUTTON_LEFT) || s_simulatePressed >= 0)
		{
			s_buttonPressed = -1;
			s32 count = (s_pdaMode == PDA_MODE_MAP) ? PDA_BTN_COUNT : PDA_BTN_EXIT + 1;
			for (s32 i = PDA_BTN_MAP; i < count; i++)
			{
				lactor_setState(s_pdaArt, 2 * (1 + i), 0);
				LRect buttonRect;
				lactorAnim_getFrame(s_pdaArt, &buttonRect);
				
				if ((s_cursorPos.x >= buttonRect.left && s_cursorPos.x < buttonRect.right &&
					s_cursorPos.z >= buttonRect.top && s_cursorPos.z < buttonRect.bottom)||
					i == s_simulatePressed)
				{
					s_buttonPressed = s32(i);
					s_buttonHover = JTRUE;
					break;
				}
			}

			if (s_pdaMode == PDA_MODE_MAP && s_buttonPressed)
			{
				switch (s_buttonPressed)
				{
					case PDA_BTN_PANUP:
					{
						automap_updateMapData(MAP_MOVE1_DN);
					} break;
					case PDA_BTN_PANRIGHT:
					{
						automap_updateMapData(MAP_MOVE1_LEFT);
					} break;
					case PDA_BTN_PANDOWN:
					{
						automap_updateMapData(MAP_MOVE1_UP);
					} break;
					case PDA_BTN_PANLEFT:
					{
						automap_updateMapData(MAP_MOVE1_RIGHT);
					} break;
					case PDA_BTN_ZOOMOUT:
					{
						automap_updateMapData(MAP_ZOOM_IN);
					} break;
					case PDA_BTN_ZOOMIN:
					{
						automap_updateMapData(MAP_ZOOM_OUT);
					} break;
					case PDA_BTN_LAYERUP:
					{
						if (s_buttonPressed == s_simulatePressed)
						{
							automap_updateMapData(MAP_LAYER_UP);
						}
					} break;
					case PDA_BTN_LAYERDOWN:
					{
						if (s_buttonPressed == s_simulatePressed)
						{
							automap_updateMapData(MAP_LAYER_DOWN);
						}
					} break;
				}
			}
		}
		else if (TFE_Input::mouseDown(MBUTTON_LEFT) && s_buttonPressed >= 0)
		{
			lactor_setState(s_pdaArt, 2 * (1 + s_buttonPressed), 0);
			LRect buttonRect;
			lactorAnim_getFrame(s_pdaArt, &buttonRect);

			// Verify that the mouse is still over the button.
			if (s_cursorPos.x >= buttonRect.left && s_cursorPos.x < buttonRect.right &&
				s_cursorPos.z >= buttonRect.top && s_cursorPos.z < buttonRect.bottom)
			{
				s_buttonHover = JTRUE;
			}
			else
			{
				s_buttonHover = JFALSE;
			}

			if (s_buttonHover && s_pdaMode == PDA_MODE_MAP)
			{
				switch (s_buttonPressed)
				{
					case PDA_BTN_PANUP:
					{
						automap_updateMapData(MAP_MOVE1_DN);
					} break;
					case PDA_BTN_PANRIGHT:
					{
						automap_updateMapData(MAP_MOVE1_LEFT);
					} break;
					case PDA_BTN_PANDOWN:
					{
						automap_updateMapData(MAP_MOVE1_UP);
					} break;
					case PDA_BTN_PANLEFT:
					{
						automap_updateMapData(MAP_MOVE1_RIGHT);
					} break;
					case PDA_BTN_ZOOMOUT:
					{
						automap_updateMapData(MAP_ZOOM_IN);
					} break;
					case PDA_BTN_ZOOMIN:
					{
						automap_updateMapData(MAP_ZOOM_OUT);
					} break;
				}
			}
		}
		else
		{
			if (s_buttonPressed >= 0 && s_buttonHover && s_simulatePressed < 0)
			{
				switch (s_buttonPressed)
				{
					case PDA_BTN_MAP:
					{
						s_pdaMode = PDA_MODE_MAP;
					} break;
					case PDA_BTN_WEAPONS:
					{
						s_pdaMode = PDA_MODE_WEAPONS;
					} break;
					case PDA_BTN_INV:
					{
						s_pdaMode = PDA_MODE_INV;
					} break;
					case PDA_BTN_GOALS:
					{
						s_pdaMode = PDA_MODE_GOALS;
					} break;
					case PDA_BTN_BRIEF:
					{
						s_pdaMode = PDA_MODE_BRIEF;
					} break;
					case PDA_BTN_EXIT:
					{
						s_pdaOpen = JFALSE;
						automap_updateMapData(MAP_ENABLE_AUTOCENTER);
					} break;
					case PDA_BTN_LAYERUP:
					{
						automap_updateMapData(MAP_LAYER_UP);
					} break;
					case PDA_BTN_LAYERDOWN:
					{
						automap_updateMapData(MAP_LAYER_DOWN);
					} break;
				}
			}
			// Reset.
			s_buttonPressed = -1;
			s_buttonHover = JFALSE;
		}
		s_simulatePressed = -1;
	}

	void pda_handleInput()
	{
		menu_handleMousePosition();
		automap_setPdaActive(JTRUE);
		
		if (TFE_Input::keyPressed(KEY_SPACE))
		{
			if (TFE_Input::keyDown(KEY_LSHIFT) || TFE_Input::keyDown(KEY_RSHIFT))
			{
				s_pdaMode = (s_pdaMode == PDA_MODE_MAP) ? PDA_MODE_BRIEF : PdaMode(s_pdaMode - 1);
			}
			else
			{
				s_pdaMode = (s_pdaMode == PDA_MODE_BRIEF) ? PDA_MODE_MAP : PdaMode(s_pdaMode + 1);
			}
		}

		if (s_pdaMode == PDA_MODE_MAP)
		{
			if (TFE_Input::keyDown(KEY_UP))
			{
				s_simulatePressed = PDA_BTN_PANUP;
			}
			else if (TFE_Input::keyDown(KEY_DOWN))
			{
				s_simulatePressed = PDA_BTN_PANDOWN;
			}
			if (TFE_Input::keyDown(KEY_LEFT))
			{
				s_simulatePressed = PDA_BTN_PANLEFT;
			}
			else if (TFE_Input::keyDown(KEY_RIGHT))
			{
				s_simulatePressed = PDA_BTN_PANRIGHT;
			}

			if (TFE_Input::keyDown(KEY_EQUALS))
			{
				s_simulatePressed = PDA_BTN_ZOOMIN;
			}
			else if (TFE_Input::keyDown(KEY_MINUS))
			{
				s_simulatePressed = PDA_BTN_ZOOMOUT;
			}

			if (TFE_Input::keyPressed(KEY_LEFTBRACKET))
			{
				s_simulatePressed = PDA_BTN_LAYERUP;
			}
			else if (TFE_Input::keyPressed(KEY_RIGHTBRACKET))
			{
				s_simulatePressed = PDA_BTN_LAYERDOWN;
			}
		}

		pda_handleButtons();
	}

	void pda_drawButton(PdaButton id)
	{
		s32 pressed = 0;
		if ((s_buttonHover && id == s_buttonPressed) || (id == s_pdaMode))
		{
			pressed = 1;
		}

		lactor_setState(s_pdaArt, 2 * (1 + id) + (pressed ? 0 : 1), 0);
		lactorAnim_draw(s_pdaArt, &s_viewBounds, &s_viewBounds, 0, 0, JTRUE);
	}

	void pda_drawCommonButtons()
	{
		for (s32 i = PDA_BTN_MAP; i <= PDA_BTN_EXIT; i++)
		{
			pda_drawButton(PdaButton(i));
		}
	}

	void pda_drawMapButtons()
	{
		for (s32 i = PDA_BTN_PANUP; i <= PDA_BTN_LAYERDOWN; i++)
		{
			pda_drawButton(PdaButton(i));
		}
	}

	void pda_update()
	{
		if (!s_pdaOpen)
		{
			return;
		}
		
		if (TFE_Input::keyPressed(KEY_F1) || TFE_Input::keyPressed(KEY_ESCAPE))
		{
			s_pdaOpen = JFALSE;
			return;
		}
		
		tfe_updateLTime();

		// Input
		pda_handleInput();

		// Main view
		if (s_pdaMode == PDA_MODE_MAP)
		{
			ScreenRect* uiRect = vfb_getScreenRect(VFB_RECT_UI);
			fixed16_16 xScale = vfb_getXScale();
			fixed16_16 yScale = vfb_getYScale();

			s32 virtualWidth = floor16(mul16(intToFixed16(320), xScale));
			s32 offset = max(0, ((uiRect->right - uiRect->left + 1) - virtualWidth) / 2);

			u32 outWidth, outHeight;
			vfb_getResolution(&outWidth, &outHeight);
			memset(vfb_getCpuBuffer(), 0, outWidth * outHeight);

			ScreenRect clipRect;
			clipRect.left  = offset + floor16(mul16(intToFixed16(s_pdaRect.left),  xScale) + xScale);
			clipRect.right = offset + floor16(mul16(intToFixed16(s_pdaRect.right), xScale) - xScale);
			clipRect.top   = floor16(mul16(intToFixed16(s_pdaRect.top), yScale) + yScale);
			clipRect.bot   = floor16(mul16(intToFixed16(s_pdaRect.bottom), yScale) - yScale);

			vfb_setScreenRect(VFB_RECT_RENDER, &clipRect);
			automap_draw(vfb_getCpuBuffer());
			vfb_restoreScreenRect(VFB_RECT_RENDER);
		}

		// Background
		lcanvas_eraseRect(&s_viewBounds);
		lactor_setState(s_pdaArt, 0, 0);
		lactorAnim_draw(s_pdaArt, &s_viewBounds, &s_viewBounds, 0, 0, JTRUE);

		// Common buttons
		pda_drawCommonButtons();
		if (s_pdaMode == PDA_MODE_MAP)
		{
			pda_drawMapButtons();
		}

		menu_blitCursor(s_cursorPos.x, s_cursorPos.z, s_framebuffer);

		screenDraw_setTransColor(79);
		menu_blitToScreen(nullptr, s_pdaMode == PDA_MODE_MAP ? JTRUE : JFALSE);
		screenDraw_setTransColor(0);
	}
	
	///////////////////////////////////////////
	// Internal Implementation
	///////////////////////////////////////////
}