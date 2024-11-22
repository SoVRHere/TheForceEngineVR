#include "scriptInterface.h"
#include <TFE_Editor/LevelEditor/Scripting/levelEditorScripts.h>
#include <TFE_System/math.h>
#include <TFE_System/system.h>
#include <TFE_Jedi/Math/core_math.h>
#include <TFE_Editor/editor.h>
#include <TFE_Editor/errorMessages.h>
#include <TFE_ForceScript/forceScript.h>
#include <TFE_Editor/LevelEditor/infoPanel.h>
#include <TFE_Editor/EditorAsset/editorTexture.h>
#include <TFE_Editor/EditorAsset/editorFrame.h>
#include <TFE_Editor/EditorAsset/editorSprite.h>
#include <TFE_Ui/ui.h>

#include <algorithm>
#include <vector>
#include <string>
#include <map>

#ifdef ENABLE_FORCE_SCRIPT
#include <angelscript.h>

using namespace TFE_Editor;
using namespace TFE_Jedi;
using namespace TFE_ForceScript;

const char* s_enumType;
const char* s_typeName;
const char* s_objVar;

namespace TFE_ScriptInterface
{
	struct ScriptToRun
	{
		std::string moduleName;
		std::string funcName;
		s32 argCount;
		ScriptArg args[TFE_MAX_SCRIPT_ARG];
	};

	static std::vector<ScriptToRun> s_scriptsToRun;
	static ScriptAPI s_api = API_UNKNOWN;
	static const char* s_searchPath = nullptr;

	void registerScriptInterface(ScriptAPI api)
	{
		switch (api)
		{
			case API_SHARED:
			{
			} break;
			case API_LEVEL_EDITOR:
			{
				LevelEditor::registerScriptFunctions(api);
			} break;
			case API_SYSTEM_UI:
			{
			} break;
			case API_GAME:
			{
			} break;
		}
	}

	template<typename... Args>
	void runScript(const char* scriptName, const char* scriptFunc, Args... inputArgs)
	{
		ScriptToRun script;
		script.moduleName = scriptName;
		script.funcName = scriptFunc;
		script.argCount = 0;
		// Unpack the function arguments and store in `script.args`.
		scriptArgIterator(script.argCount, script.args, inputArgs...);

		s_scriptsToRun.push_back(script);
	}

	void runScript(const char* scriptName, const char* scriptFunc)
	{
		s_scriptsToRun.push_back({ scriptName, scriptFunc, 0 });
	}

	void recompileScript(const char* scriptName)
	{
		// TODO
	}

	void setAPI(ScriptAPI api, const char* searchPath)
	{
		if (api != API_UNKNOWN)
		{
			s_api = ScriptAPI(api | API_SHARED);
		}
		s_searchPath = searchPath;
	}
		
	void update()
	{
		if (s_api == API_UNKNOWN) { return; }

		char scriptPath[TFE_MAX_PATH];
		const s32 count = (s32)s_scriptsToRun.size();
		const ScriptToRun* script = s_scriptsToRun.data();
		for (s32 i = 0; i < count; i++, script++)
		{
			// Does the module already exist?
			const char* moduleName = script->moduleName.c_str();
			TFE_ForceScript::ModuleHandle scriptMod = TFE_ForceScript::getModule(moduleName);
			// If not than try to load and compile it.
			if (!scriptMod)
			{
				if (s_searchPath)
				{
					sprintf(scriptPath, "%s/%s.fs", s_searchPath, moduleName);
					scriptMod = TFE_ForceScript::createModule(moduleName, scriptPath, false, s_api);
				}
				// Try loading from the archives and full search paths.
				if (!scriptMod)
				{
					sprintf(scriptPath, "%s.fs", moduleName);
					scriptMod = TFE_ForceScript::createModule(moduleName, scriptPath, true, s_api);

				}
			}
			if (scriptMod)
			{
				TFE_ForceScript::FunctionHandle func = TFE_ForceScript::findScriptFuncByName(scriptMod, script->funcName.c_str());
				TFE_ForceScript::execFunc(func, script->argCount, script->args);
			}
		}
		s_scriptsToRun.clear();
	}
}
#else
namespace LevelEditor
{
	void registerScriptFunctions() {}

	void executeLine(const char* line) {}
	void runLevelScript(const char* scriptName) {}
	void showLevelScript(const char* scriptName) {}

	void levelScript_update() {}
}
#endif
