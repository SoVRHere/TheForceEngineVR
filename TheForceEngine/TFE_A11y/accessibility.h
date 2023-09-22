#pragma once
#include <string>
#include <vector>
#include <TFE_System/system.h>
#include <filePathList.h>
using std::string;

namespace TFE_A11Y {
	///////////////////////////////////////////
	// Enums/structs
	///////////////////////////////////////////
	enum A11yStatus
	{
		CC_NOT_LOADED, CC_LOADED, CC_ERROR
	};

	enum CaptionEnv 
	{
		CC_GAMEPLAY, CC_CUTSCENE
	};

	enum CaptionType
	{
		CC_VOICE, CC_EFFECT
	};

	struct Caption 
	{
		string text;
		s64 microsecondsRemaining;
		CaptionType type;
		CaptionEnv env;
	};

	///////////////////////////////////////////
	// Constants
	///////////////////////////////////////////
	const string FILE_NAME_START = "subtitles-";
	const string FILE_NAME_EXT = ".txt";

	///////////////////////////////////////////
	// Functions
	///////////////////////////////////////////
	void init();

	// Fonts
	FilePathList getFontFiles();
	FilePath getCurrentFontFile();
	void setPendingFont(const string path);
	bool hasPendingFont();
	void loadPendingFont();

	// Captions
	FilePathList getCaptionFiles();
	FilePath getCurrentCaptionFile();
	A11yStatus getStatus();
	void loadCaptions(const string path);
	void clearActiveCaptions();
	void drawCaptions();
	void drawExampleCaptions();
	void focusCaptions();
	void enqueueCaption(Caption caption);
	void onSoundPlay(char* name, CaptionEnv env);

	// True if captions or subtitles are enabled for cutscenes
	bool cutsceneCaptionsEnabled();
	// True if captions or subtitles are enabled for gameplay
	bool gameplayCaptionsEnabled();
}