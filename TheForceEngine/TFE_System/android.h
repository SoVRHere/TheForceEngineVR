#pragma once

#include <TFE_System/system.h>
#include <android/asset_manager.h>
#include <android/native_activity.h>
#include <string>

namespace TFE_System::android
{
	AAssetManager*	GetAAssetManager();
	jobject 		GetActivity();
	JNIEnv* 		GetJNIEnv();
	void 			Log(LogWriteType type, const char* message);
}
