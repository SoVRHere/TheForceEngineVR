#pragma once
//////////////////////////////////////////////////////////////////////
// The Force Engine System Library
// System functionality, such as timers and logging.
//////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <string>
#include "types.h"

#if defined(ANDROID)
#include <android/log.h>
#endif

#define TFE_MAJOR_VERSION 1
#define TFE_MINOR_VERSION 10
#define TFE_BUILD_VERSION 0

enum LogWriteType
{
	LOG_MSG = 0,
	LOG_WARNING,
	LOG_ERROR,
	LOG_CRITICAL,		//Critical log messages are CRITICAL errors that also act as asserts when attached to a debugger.
	LOG_COUNT
};

namespace TFE_System
{
	void init(f32 refreshRate, bool synced, const char* versionString);
	void shutdown();
	void resetStartTime();
	void setVsync(bool sync);
	bool getVSync();

	void update();
	f64 updateThreadLocal(u64* localTime);

	// Timing
	// --- The current time and delta time are determined once per frame, during the update() function.
	//     In other words an entire frame operates on a single instance of time.
	// Return the delta time.
	f64 getDeltaTime();
	f64 getDeltaTimeRaw();
	// Get the absolute time since the last start time, in seconds.
	f64 getTime();

	u64 getCurrentTimeInTicks();
	f64 convertFromTicksToSeconds(u64 ticks);
	f64 convertFromTicksToMillis(u64 ticks);
	f64 microsecondsToSeconds(f64 mu);
	u64 getStartTime();
	void setStartTime(u64 startTime);

	void getDateTimeString(char* output);
	void getDateTimeStringForFile(char* output);

	// Log
	void logTimeToggle();
	bool logOpen(const char* filename, bool append=false);
	void logClose();
	void logWrite(LogWriteType type, const char* tag, const char* str, ...);

	// Lighter weight debug output (only useful when running in a terminal or debugger).
	void debugWrite(const char* tag, const char* str, ...);

	// System
	bool osShellExecute(const char* pathToExe, const char* exeDir, const char* param, bool waitForCompletion);
	void postErrorMessageBox(const char* msg, const char* title);
	void sleep(u32 sleepDeltaMS);

	void postQuitMessage();
	bool quitMessagePosted();

	void postSystemUiRequest();
	bool systemUiRequestPosted();

	const char* getVersionString();

	bool openURL(const std::string& url);

	extern f64 c_gameTimeScale;

	void setFrame(u32 frame);
	u32 getFrame();
}

// _strlwr/_strupr exist on Windows; roll our own
// and use them throghout. This works on Windows
// and Linux/unix-like.
static inline void __strlwr(char *c)
{
	while (*c) {
		*c = tolower(*c);
		c++;
	}
}

static inline void __strupr(char *c)
{
	while (*c) {
		*c = toupper(*c);
		c++;
	}
}

#define FMT_HEADER_ONLY
#include "fmt/format.h"

#define TFE_CRITICAL(tag, ...) TFE_System::logWrite(LOG_CRITICAL, tag, fmt::format(__VA_ARGS__).c_str())
#define TFE_ERROR(tag, ...) TFE_System::logWrite(LOG_ERROR, tag, fmt::format(__VA_ARGS__).c_str())
#define TFE_WARN(tag, ...) TFE_System::logWrite(LOG_WARNING, tag, fmt::format(__VA_ARGS__).c_str())
#define TFE_INFO(tag, ...) TFE_System::logWrite(LOG_MSG, tag, fmt::format(__VA_ARGS__).c_str())

#define TFE_ASSERT assert

#if defined(ANDROID)
#define TFE_ANDROID(...) __android_log_print(ANDROID_LOG_DEBUG, "TFE", fmt::format(__VA_ARGS__).c_str())
#else
#define TFE_ANDROID(...) 
#endif

template<typename T>
class ScopeFunction final
{
public:
	explicit ScopeFunction(T outCallable)
		: mOutCallable{ std::move(outCallable) }
	{}

	ScopeFunction(ScopeFunction&&) = delete;

	~ScopeFunction()
	{
		mOutCallable();
	}

private:
	T mOutCallable;
};

template<typename U, typename T>
class ScopeFunctions final
{
public:
	ScopeFunctions(U inCallable, T outCallable)
		: mOutCallable{ std::move(outCallable) }
	{
		inCallable();
	}

	ScopeFunctions(ScopeFunctions&&) = delete;

	~ScopeFunctions()
	{
		mOutCallable();
	}

private:
	T mOutCallable;
};

template<typename T>
void SafeDelete(T*& p)
{
	if (p != nullptr)
	{
		delete p;
		p = nullptr;
	}
}

template<typename T>
auto EnumValue(T e)
{
	return std::underlying_type_t<T>();
}

template<> struct fmt::formatter<Vec2f> : formatter<string_view> { auto format(const Vec2f& v, format_context& ctx) { return fmt::format_to(ctx.out(), "[{: 6.2}, {: 6.2}]", v.x, v.y); } };
template<> struct fmt::formatter<Vec3f> : formatter<string_view> { auto format(const Vec3f& v, format_context& ctx) { return fmt::format_to(ctx.out(), "[{: 6.2}, {: 6.2}, {: 6.2}]", v.x, v.y, v.z); } };
template<> struct fmt::formatter<Vec4f> : formatter<string_view> { auto format(const Vec4f& v, format_context& ctx) { return fmt::format_to(ctx.out(), "[{: 6.2}, {: 6.2}, {: 6.2}, {: 6.2}]", v.x, v.y, v.z, v.w); } };
