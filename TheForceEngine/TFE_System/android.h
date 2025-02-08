#pragma once

#include <SDL.h>
#include <TFE_System/system.h>
#include <TFE_Game/igame.h>
#include <android/asset_manager.h>
#include <android/native_activity.h>

namespace TFE_System::android
{
	AAssetManager*	GetAAssetManager();
	jobject 		GetActivity();
	JNIEnv* 		GetJNIEnv();
	void 			Log(LogWriteType type, const char* message);
	const std::string& GetExternalStorageDir();
}
