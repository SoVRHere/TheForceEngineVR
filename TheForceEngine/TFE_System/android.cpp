#include "android.h"

#include <jni.h>
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <SDL_system.h>  // for SDL_AndroidGetExternalStoragePath, SDL_AndroidGetJNIEnv and SDL_AndroidGetActivity
#include <filesystem>
#include <vector>
#include <fstream>
#include <cassert>

#define TFE_JAVA_PREFIX	com_tfe_core
#define CONCAT1(prefix, class, function) CONCAT2(prefix, class, function)
#define CONCAT2(prefix, class, function) Java_ ## prefix ## _ ## class ## _ ## function
#define TFE_JAVA_INTERFACE(function) CONCAT1(TFE_JAVA_PREFIX, AndroidActivity, function)

extern "C"
{
	JNIEXPORT void JNICALL TFE_JAVA_INTERFACE(onCreateActivity)(JNIEnv* env, jclass jcls, jobject activity, jobject assetManager, jobject assetList);
	JNIEXPORT void JNICALL TFE_JAVA_INTERFACE(onDestroyActivity)(JNIEnv* env, jclass jcls);
}

#define LOGI(fmt, ...) __android_log_print(ANDROID_LOG_INFO, "TFE", fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) __android_log_print(ANDROID_LOG_DEBUG, "TFE", fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) __android_log_print(ANDROID_LOG_WARN, "TFE", fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) __android_log_print(ANDROID_LOG_ERROR, "TFE", fmt, ##__VA_ARGS__)

// The caller is responsible for obtaining and holding a VM reference to the jobject
// to prevent it being garbage collected while the native object is in use.
static jobject gJavaAssetManagerRef = 0;
static AAssetManager* gAAssetManager = nullptr;

static jobject gActivityRef = 0;

namespace fs = std::filesystem;

static void CopyNewAssets(AAssetManager* assetManager, const std::vector<std::string>& assetItems)
{
	const fs::path storagePath{ SDL_AndroidGetExternalStoragePath() };
	std::vector<uint8_t> buffer;

	for (const std::string& assetItem : assetItems)
	{
		//LOGI("assetItem: %s", assetItem.c_str());
		const fs::path assetPath = storagePath / assetItem;
		if (!fs::exists(assetPath))
		{
			fs::path dir{ assetPath };
			dir.remove_filename();
			if (!fs::exists(dir))
			{
				fs::create_directories(dir);
			}

			LOGI("copying new asset to %s", assetPath.c_str());
			if (AAsset* asset = AAssetManager_open(assetManager, assetItem.c_str(), AASSET_MODE_BUFFER); asset)
			{
				std::ofstream assetFileOut{ assetPath, std::ios::binary | std::ios::trunc };
				if (assetFileOut.is_open())
				{
					const size_t fileLength = AAsset_getLength(asset);
					buffer.resize(fileLength);
					if (int read = AAsset_read(asset, buffer.data(), fileLength); read >= 0)
					{
						if (!assetFileOut.write((const char *)buffer.data(), buffer.size()))
						{
							LOGE("writing asset %s", assetPath.c_str());
						}
					}
					else
					{
						LOGE("reading asset %s", assetItem.c_str());
					}
				}
				else
				{
					LOGE("can't create asset file %s", assetPath.c_str());
				}

				AAsset_close(asset);
			}
			else
			{
				LOGE("opening asset %s", assetItem.c_str());
			}
		}
	}
}

static std::vector<std::string> ListString2Cpp(JNIEnv* env, jobject arrayList)
{
	jclass java_util_ArrayList = static_cast<jclass>(env->NewGlobalRef(env->FindClass("java/util/ArrayList")));
	jmethodID java_util_ArrayList_size = env->GetMethodID(java_util_ArrayList, "size", "()I");
	jmethodID java_util_ArrayList_get = env->GetMethodID(java_util_ArrayList, "get", "(I)Ljava/lang/Object;");

	jint len = env->CallIntMethod(arrayList, java_util_ArrayList_size);
	std::vector<std::string> result;
	result.reserve(len);
	for (jint i = 0; i < len; i++)
	{
		jstring element = static_cast<jstring>(env->CallObjectMethod(arrayList, java_util_ArrayList_get, i));
		const char* pchars = env->GetStringUTFChars(element, nullptr);
		result.emplace_back(pchars);
		env->ReleaseStringUTFChars(element, pchars);
		env->DeleteLocalRef(element);
	}

	return result;
}

JNIEXPORT void JNICALL TFE_JAVA_INTERFACE(onCreateActivity)(JNIEnv* env, jclass jcls, jobject activity, jobject assetManager, jobject assetList)
{
	TFE_ASSERT((gJavaAssetManagerRef == 0) && "Java's asset manager was not previously destroyed");
	TFE_ASSERT((gAAssetManager == nullptr) && "AAssetManager was not previously destroyed");
	TFE_ASSERT((gActivityRef == 0) && "Activity was not previously destroyed");

	// create a global reference to java's asset manager, so that the garbage collector
	// won't release it and that the native object remains valid
	gJavaAssetManagerRef = env->NewGlobalRef(assetManager);

	// get the Android C++ wrapper for the Java's asset manager
	gAAssetManager = AAssetManager_fromJava(env, assetManager);

	gActivityRef = env->NewGlobalRef(activity);

	CopyNewAssets(gAAssetManager, ListString2Cpp(env, assetList));
}

JNIEXPORT void JNICALL TFE_JAVA_INTERFACE(onDestroyActivity)(JNIEnv* env, jclass jcls)
{
	gAAssetManager = nullptr;
	env->DeleteGlobalRef(gJavaAssetManagerRef);
	gJavaAssetManagerRef = 0;
	env->DeleteGlobalRef(gActivityRef);
	gActivityRef = 0;
}

namespace TFE_System::android
{
	AAssetManager* GetAAssetManager()
	{
		return gAAssetManager;
	}

	jobject GetActivity()
	{
		return gActivityRef;
	}

	// thread specific environment
	JNIEnv* GetJNIEnv()
	{
		return (JNIEnv*)SDL_AndroidGetJNIEnv();
	}

	void Log(LogWriteType type, const char* message)
	{
		if (!message)
		{
			return;
		}

		auto GetAndroidLogType = [&type]() -> android_LogPriority {
			switch (type)
			{
			case LOG_MSG:
				return ANDROID_LOG_INFO;
			case LOG_WARNING:
				return ANDROID_LOG_WARN;
			case LOG_ERROR:
				return ANDROID_LOG_ERROR;
			case LOG_CRITICAL:
				return ANDROID_LOG_FATAL;
			default:
				return ANDROID_LOG_UNKNOWN;
			}
		};
		__android_log_print(GetAndroidLogType(), "TFE", "%s", message);
	}
}
