#pragma once
//////////////////////////////////////////////////////////////////////
// The Force Engine Editor
// A system built to view and edit Dark Forces data files.
// The viewing aspect needs to be put in place at the beginning
// in order to properly test elements in isolation without having
// to "play" the game as intended.
//////////////////////////////////////////////////////////////////////
#include <TFE_System/types.h>
#include <TFE_Editor/EditorAsset/editorAsset.h>
#include "selection.h"

namespace LevelEditor
{
	struct EditorSector;

	enum LevelEditFlags
	{
		LEF_NONE = 0,
		LEF_SHOW_GRID = FLAG_BIT(0),
		LEF_SHOW_LOWER_LAYERS = FLAG_BIT(1),
		LEF_SHOW_ALL_LAYERS = FLAG_BIT(2),
		LEF_SHOW_INF_COLORS = FLAG_BIT(3),
		LEF_FULLBRIGHT = FLAG_BIT(4),

		LEF_SECTOR_EDGES = FLAG_BIT(16),

		LEF_DEFAULT = LEF_SHOW_GRID | LEF_SHOW_LOWER_LAYERS | LEF_SHOW_INF_COLORS
	};

	// Level Windows open.
	enum LevelWindows
	{
		LWIN_NONE = 0,
		LWIN_HISTORY = FLAG_BIT(0),
		LWIN_VIEW_SETTINGS = FLAG_BIT(1),
	};

	bool init(TFE_Editor::Asset* asset);
	void destroy();

	void update();
	bool menu();

	void selectNone();

	// Shared Edit Commands
	void edit_moveVertices(s32 count, const FeatureId* vtxIds, Vec2f delta);
	void edit_moveFlats(s32 count, const FeatureId* flatIds, f32 delta);
	void edit_setVertexPos(FeatureId id, Vec2f pos);
	void edit_splitWall(s32 sectorId, s32 wallIndex, Vec2f newPos);
	void edit_deleteVertex(s32 sectorId, s32 vertexIndex);
	void edit_clearSelections();
}
