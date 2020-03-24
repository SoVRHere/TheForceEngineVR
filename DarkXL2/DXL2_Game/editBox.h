#pragma once
//////////////////////////////////////////////////////////////////////
// DarkXL 2 Game Level
// Manages the game while playing levels in first person.
// This holds one or more players, handles input updates, etc.
//////////////////////////////////////////////////////////////////////

#include <DXL2_System/types.h>

struct EditBox
{
	char* inputField = nullptr;
	s32   cursor = 0;
	s32   maxLen = 0;
};

struct Font;
class DXL2_Renderer;

namespace DXL2_GameUi
{
	void updateEditBox(EditBox* editBox);
	void drawEditBox(EditBox* editBox, Font* font, s32 x0, s32 y0, s32 x1, s32 y1, s32 scaleX, s32 scaleY, DXL2_Renderer* renderer);
}
