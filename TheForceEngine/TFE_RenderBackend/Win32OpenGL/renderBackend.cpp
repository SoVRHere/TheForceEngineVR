#include <TFE_System/math.h>
#include <TFE_RenderBackend/renderBackend.h>
#include <TFE_RenderBackend/dynamicTexture.h>
#include <TFE_RenderBackend/textureGpu.h>
#include <TFE_RenderBackend/Win32OpenGL/openGL_Caps.h>
#include <TFE_RenderBackend/Win32OpenGL/openGL_Debug.h>
#include <TFE_Settings/settings.h>
#include <TFE_Ui/ui.h>
#include <TFE_Asset/imageAsset.h>	// For image saving, this should be refactored...
#include <TFE_System/profiler.h>
#include <TFE_PostProcess/blit.h>
#include <TFE_PostProcess/bloomThreshold.h>
#include <TFE_PostProcess/bloomDownsample.h>
#include <TFE_PostProcess/bloomMerge.h>
#include <TFE_PostProcess/postprocess.h>
#include <TFE_Vr/vr.h>
#include "renderTarget.h"
#include "screenCapture.h"
#include <SDL.h>
#include "gl.h"
#include <stdio.h>
#include <assert.h>
#include <algorithm>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#undef min
#undef max

//#pragma comment(lib, "opengl32.lib") // TODO: why linker is not complaining?
#endif

namespace TFE_Jedi
{
	extern Mat3  s_cameraMtx;
	extern Mat4  s_cameraProj;
	extern Vec3f s_cameraPos;
}

namespace TFE_RenderBackend
{
	static const f32 c_tallScreenThreshold = 1.32f;	// 4:3 + epsilon.

	// Screenshot stuff... needs to be refactored.
	static char s_screenshotPath[TFE_MAX_PATH];
	static bool s_screenshotQueued = false;

	static WindowState m_windowState;
	static void* m_window;
	static DynamicTexture* s_virtualDisplay = nullptr;
	static DynamicTexture* s_palette = nullptr;
	static u32 s_paletteCpu[256];
	static TextureGpu* s_virtualRenderTexture = nullptr;
	static TextureGpu* s_virtualRenderDepthTexture = nullptr;
	static TextureGpu* s_materialRenderTexture = nullptr;
	static RenderTarget* s_virtualRenderTarget = nullptr;
	static ScreenCapture*  s_screenCapture = nullptr;

	static RenderTarget* s_copyTarget = nullptr;

	static u32 s_virtualWidth, s_virtualHeight;
	static u32 s_virtualWidthUi;
	static u32 s_virtualWidth3d;

	static bool s_widescreen = false;
	static bool s_asyncFrameBuffer = true;
	static bool s_gpuColorConvert = false;
	static bool s_useRenderTarget = false;
	static bool s_bloomEnable = false;
	static DisplayMode s_displayMode;
	static f32 s_clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	static u32 s_rtWidth, s_rtHeight;

	static Blit* s_postEffectBlit;
	static BloomThreshold* s_bloomTheshold;
	static BloomDownsample* s_bloomDownsample;
	static BloomMerge* s_bloomMerge;
	static std::vector<SDL_Rect> s_displayBounds;

#if defined(ENABLE_VR)
	vr::UpdateStatus s_VRUpdateStatus = vr::UpdateStatus::ShouldNotRender;
	Mat4  s_cameraProjVR[2];
	Mat4  s_cameraProjVR_YDown[2];
	Mat3  s_cameraMtxVR[2];
	Mat3  s_cameraMtxVR_YDown[2];
	Vec3f s_cameraPosVR[2];
	Vec3f s_cameraPosVR_YDown[2];
#endif

	void drawVirtualDisplay();
	void setupPostEffectChain(bool useDynamicTexture, bool useBloom);
	bool initVR();

	static void printGLInfo(void)
	{
		const char* gl_ver = (const char *)glGetString(GL_VERSION);
		if (!gl_ver || glGetError() != GL_NO_ERROR)
		{
			TFE_System::logWrite(LOG_ERROR, "RenderBackend", "cannot get GL Version!");
			return;
		}
		const char* gl_ren = (const char *)glGetString(GL_RENDERER);
		TFE_System::logWrite(LOG_MSG, "RenderBackend", "GL Info: %s, %s", gl_ver, gl_ren);
	}
		
	SDL_Window* createWindow(const WindowState& state)
	{
		u32 windowFlags = SDL_WINDOW_OPENGL;
		bool windowed = !(state.flags & WINFLAG_FULLSCREEN);

		TFE_Settings_Window* windowSettings = TFE_Settings::getWindowSettings();
		
		s32 x = windowSettings->x, y = windowSettings->y;
		s32 displayIndex = getDisplayIndex(x, y);
		assert(displayIndex >= 0);
		
		if (windowed)
		{
			y = std::max(32, y);
			windowSettings->y = y;
			windowFlags |= SDL_WINDOW_RESIZABLE;
		}
		else
		{
			MonitorInfo monitorInfo;
			getDisplayMonitorInfo(displayIndex, &monitorInfo);

			x = monitorInfo.x;
			y = monitorInfo.y;
			windowFlags |= SDL_WINDOW_BORDERLESS;
		}

#ifdef _DEBUG
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);

#if defined(ANDROID)
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif

		TFE_System::logWrite(LOG_MSG, "RenderBackend", "SDL Videodriver: %s", SDL_GetCurrentVideoDriver());
		SDL_Window* window = SDL_CreateWindow(state.name, x, y, state.width, state.height, windowFlags);
		if (!window)
		{
			TFE_System::logWrite(LOG_ERROR, "RenderBackend", "SDL_CreateWindow() failed: %s", SDL_GetError());
			return nullptr;
		}

		SDL_GLContext context = SDL_GL_CreateContext(window);
		if (!context)
		{
			SDL_DestroyWindow(window);
			TFE_System::logWrite(LOG_ERROR, "RenderBackend", "SDL_GL_CreateContext() failed: %s", SDL_GetError());
			return nullptr;
		}

#if defined(ANDROID)
		int glver = gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress);
#else
		int glver = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress);
#endif
		if (glver == 0)
		{
			TFE_System::logWrite(LOG_ERROR, "RenderBackend", "cannot initialize GLAD");
			SDL_GL_DeleteContext(context);
			SDL_DestroyWindow(window);
			return nullptr;
		}

		OpenGL_Caps::queryCapabilities();
		printGLInfo();
		int tier = OpenGL_Caps::getDeviceTier();
		TFE_System::logWrite(LOG_MSG, "RenderBackend", "OpenGL Device Tier: %d", tier);
		if (tier < 2)
		{
			TFE_System::logWrite(LOG_ERROR, "RenderBackend", "Insufficient GL capabilities!");
			SDL_GL_DeleteContext(context);
			SDL_DestroyWindow(window);
			return nullptr;
		}

		OpenGL_Debug::Initialize();
		const bool inVr = initVR();
		//swap buffer at the monitors rate
		SDL_GL_SetSwapInterval((state.flags & WINFLAG_VSYNC) ? 1 : 0);

		MonitorInfo monitorInfo;
		getDisplayMonitorInfo(displayIndex, &monitorInfo);
		// High resolution displays (> 1080p) tend to be very high density, so increase the scale somewhat.
		s32 uiScale = 100;
		if (monitorInfo.h >= 2160) // 4k+
		{
			uiScale = 125 * monitorInfo.h / 1080;	// scale based on 1080p being the base.
		}
		else if (monitorInfo.h >= 1440) // 1440p
		{
			uiScale = 150;
		}
#if defined(ANDROID)
#if defined(ENABLE_VR)
		uiScale = 200;
#else
		uiScale = 225;

		float dpi = -1.0f;
		if (SDL_GetDisplayDPI(0, nullptr, &dpi, nullptr) == 0 && dpi > 0.0f)
		{
			uiScale = (s32)(uiScale * dpi / 400.0f);
		}
#endif
#endif

	#ifndef _WIN32
		SDL_SetWindowFullscreen(window, windowed ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
	#endif

		TFE_Ui::init(window, context, uiScale);
		return window;
	}

	bool initVR()
	{
		bool& inVr = TFE_Settings::getTempSettings()->vr;
#if defined(ENABLE_VR)
		if (inVr)
		{
			if (!vr::IsInitialized())
			{
				bool& useMultiview = TFE_Settings::getTempSettings()->vrMultiview;
				useMultiview = true; // force multiview for now
				// NOTE: looks like we cannot use GL_OVR_multiview as we need multiview uniforms in fragment shaders e.g. blit.frag, gpu_render_modelSolid.frag
				if (!SDL_GL_ExtensionSupported("GL_OVR_multiview2"))
				{
					useMultiview = false;
					inVr = false;
					TFE_WARN("VR", "\"GL_OVR_multiview2\" extension is required but not supported.");
					//if (useMultiview)
					//{
					//	TFE_WARN("VR", "Multiview rendering requested but \"GL_OVR_multiview2\" extension is not supported, using dual pass rendering.");
					//}
				}

				if (inVr && vr::Initialize(useMultiview))
				{
				}
				else
				{
					inVr = false;
					TFE_WARN("VR", "Failed to initialize VR, fallback to non VR mode.");
				}
			}
		}
#endif

		return inVr;
	}
		
	bool init(const WindowState& state)
	{
		m_window = createWindow(state);
		m_windowState = state;
		if (!m_window)
			return false;

		if (!TFE_PostProcess::init())
		{
			SDL_DestroyWindow((SDL_Window *)m_window);
			return false;
		}
		// TODO: Move effect creation into post effect system.
		s_postEffectBlit = new Blit();
		s_postEffectBlit->init();
		s_postEffectBlit->enableFeatures(BLIT_GPU_COLOR_CONVERSION);

		bool optimized = TFE_Settings::getGraphicsSettings()->bloomUseOptimizedShaders;

		s_bloomTheshold = new BloomThreshold{ optimized };
		s_bloomTheshold->init();

		s_bloomDownsample = new BloomDownsample{ optimized };
		s_bloomDownsample->init();

		s_bloomMerge = new BloomMerge{ optimized /*TODO: ignored inside*/};
		s_bloomMerge->init();
		
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClearDepthf(0.0f);

		s_palette = new DynamicTexture();
		s_palette->create(256, 1, 2);

		s_screenCapture = new ScreenCapture();
#if defined(ENABLE_VR)
		if (TFE_Settings::getTempSettings()->vr)
		{
			s_screenCapture->create(vr::GetRenderTargetSize().x, vr::GetRenderTargetSize().y, 4);
		}
		else
#endif
		{
			s_screenCapture->create(m_windowState.width, m_windowState.height, 4);
		}

		TFE_RenderState::clear();

		return true;
	}

	void destroy()
	{
		delete s_screenCapture;
		s_screenCapture = nullptr;

		// TODO: Move effect destruction into post effect system.
		s_postEffectBlit->destroy();
		delete s_postEffectBlit;
		s_postEffectBlit = nullptr;

		s_bloomTheshold->destroy();
		delete s_bloomTheshold;
		s_bloomTheshold = nullptr;

		s_bloomDownsample->destroy();
		delete s_bloomDownsample;
		s_bloomDownsample = nullptr;

		s_bloomMerge->destroy();
		delete s_bloomMerge;
		s_bloomMerge = nullptr;

		TFE_PostProcess::destroy();
		TFE_Ui::shutdown();

		delete s_virtualDisplay;
		delete s_virtualRenderTarget;
		delete s_virtualRenderTexture;
		delete s_virtualRenderDepthTexture;
		delete s_materialRenderTexture;
		SDL_DestroyWindow((SDL_Window*)m_window);

		s_virtualDisplay = nullptr;
		s_virtualRenderTarget = nullptr;
		s_virtualRenderTexture = nullptr;
		s_virtualRenderDepthTexture = nullptr;
		s_materialRenderTexture = nullptr;
		m_window = nullptr;
	}

	void* getWindow()
	{
		return m_window;
	}

	bool getVsyncEnabled()
	{
		return SDL_GL_GetSwapInterval() > 0;
	}

	void enableVsync(bool enable)
	{
		SDL_GL_SetSwapInterval(enable ? 1 : 0);
	}

	void pushGroup(const char* label)
	{
		OpenGL_Debug::PushGroup(label);
	}

	void popGroup()
	{
		OpenGL_Debug::PopGroup();
	}

	void setClearColor(const f32* color)
	{
		glClearColor(color[0], color[1], color[2], color[3]);
		glClearDepthf(0.0f);

		memcpy(s_clearColor, color, sizeof(f32) * 4);
	}
		
	void swap(bool blitVirtualDisplay)
	{
		// Blit the texture or render target to the screen.
		if (blitVirtualDisplay) { drawVirtualDisplay(); }
		else { glClear(GL_COLOR_BUFFER_BIT); }

		// Handle the UI.
		TFE_ZONE_BEGIN(systemUi, "System UI");
		pushGroup("System UI");
		if (TFE_Ui::isGuiFrameActive() && s_screenCapture->wantsToDrawGui()) { s_screenCapture->drawGui(); }
		TFE_Ui::render();
		// Reset the state due to UI changes.
		TFE_RenderState::clear();
		popGroup();
		TFE_ZONE_END(systemUi);

#if defined(ENABLE_VR)
		if (TFE_Settings::getTempSettings()->vr && s_VRUpdateStatus == vr::UpdateStatus::Ok)
		{
			// in VR we have to make screenshot before vr::SubmitFrame to avoid getting glReadPixels error:
			// 'GL_INVALID_FRAMEBUFFER_OPERATION error generated. Operation is not valid because a bound framebuffer is not framebuffer complete.',
			// looks like vr::SubmitFrame is changing some texture and/or framebuffer state
			if (s_screenshotQueued)
			{
				s_screenshotQueued = false;
				s_screenCapture->captureFrame(s_screenshotPath);
			}
			s_screenCapture->update();
		}
#endif

		TFE_ZONE_BEGIN(swapGpu, "GPU Swap Buffers");
		// Update the window.
#if defined(ENABLE_VR)
		if (TFE_Settings::getTempSettings()->vr)
		{
			if (s_VRUpdateStatus == vr::UpdateStatus::Ok)
			{
				vr::Commit(vr::Side::Left);
				vr::SubmitFrame();
				TFE_RenderBackend::s_VRUpdateStatus = vr::UpdateStatus::ShouldNotRender;
				//TFE_RenderBackend::invalidateRenderTarget(); TODO:
			}
		}
		else // to be able to capture frame in NVIDIA Nsight we have to swap buffers or change Frame Delimiter in Nsight settings
#endif
		{
			SDL_GL_SwapWindow((SDL_Window*)m_window);
		}
		TFE_ZONE_END(swapGpu);

		if (!TFE_Settings::getTempSettings()->vr)
		{
			if (s_screenshotQueued)
			{
				s_screenshotQueued = false;
				s_screenCapture->captureFrame(s_screenshotPath);
			}
			s_screenCapture->update();
		}
	}

	void captureScreenToMemory(u32* mem)
	{
		s_screenCapture->captureFrontBufferToMemory(mem);
	}

	void queueScreenshot(const char* screenshotPath)
	{
		strcpy(s_screenshotPath, screenshotPath);
		s_screenshotQueued = true;
	}
		
	void startGifRecording(const char* path, bool skipCountdown)
	{
		s_screenCapture->beginRecording(path, skipCountdown);
	}

	void stopGifRecording()
	{
		s_screenCapture->endRecording();
	}

	void updateSettings()
	{
		TFE_Settings_Window* windowSettings = TFE_Settings::getWindowSettings();
		if (!(m_windowState.flags & WINFLAG_FULLSCREEN))
		{
			SDL_GetWindowPosition((SDL_Window*)m_window, &windowSettings->x, &windowSettings->y);
		}
	}

	void resize(s32 width, s32 height)
	{
		TFE_Settings_Window* windowSettings = TFE_Settings::getWindowSettings();

		// in VR we do not want to change window size
		if (!TFE_Settings::getTempSettings()->vr)
		{
			m_windowState.width = width;
			m_windowState.height = height;

			windowSettings->width = width;
			windowSettings->height = height;
			if (!(m_windowState.flags & WINFLAG_FULLSCREEN))
			{
				m_windowState.baseWindowWidth = width;
				m_windowState.baseWindowHeight = height;

				windowSettings->baseWidth = width;
				windowSettings->baseHeight = height;
			}
		}
		glViewport(0, 0, width, height);
		setupPostEffectChain(!s_useRenderTarget, s_bloomEnable);

		s_screenCapture->resize(width, height);
	}

	void enumerateDisplays()
	{
		// Get the displays and their bounds.
		s32 displayCount = SDL_GetNumVideoDisplays();
		s_displayBounds.resize(displayCount);
		for (s32 i = 0; i < displayCount; i++)
		{
			SDL_GetDisplayBounds(i, &s_displayBounds[i]);
		}
	}
		
	s32 getDisplayCount()
	{
		enumerateDisplays();
		return (s32)s_displayBounds.size();
	}

	s32 getDisplayIndex(s32 x, s32 y)
	{
		enumerateDisplays();

		// Then determine which display the window sits in.
		s32 displayIndex = -1;
		for (size_t i = 0; i < s_displayBounds.size(); i++)
		{
			if (x >= s_displayBounds[i].x && x < s_displayBounds[i].x + s_displayBounds[i].w &&
				y >= s_displayBounds[i].y && y < s_displayBounds[i].y + s_displayBounds[i].h)
			{
				displayIndex = s32(i);
				break;
			}
		}

		return displayIndex;
	}
		
	bool getDisplayMonitorInfo(s32 displayIndex, MonitorInfo* monitorInfo)
	{
#if defined(ENABLE_VR)
		if (TFE_Settings::getTempSettings()->vr)
		{
			*monitorInfo = { 0, 0, (s32)vr::GetRenderTargetSize().x, (s32)vr::GetRenderTargetSize().y };
			return true;
		}
#endif

		enumerateDisplays();
		if (displayIndex >= (s32)s_displayBounds.size())
		{
			return false;
		}

		monitorInfo->x = s_displayBounds[displayIndex].x;
		monitorInfo->y = s_displayBounds[displayIndex].y;
		monitorInfo->w = s_displayBounds[displayIndex].w;
		monitorInfo->h = s_displayBounds[displayIndex].h;
		return true;
	}

	f32 getDisplayRefreshRate()
	{
		s32 x, y;
		SDL_GetWindowPosition((SDL_Window*)m_window, &x, &y);
		s32 displayIndex = getDisplayIndex(x, y);
		if (displayIndex >= 0)
		{
			SDL_DisplayMode mode = { 0 };
			SDL_GetDesktopDisplayMode(displayIndex, &mode);
			return (f32)mode.refresh_rate;
		}
		return 0.0f;
	}

	void getCurrentMonitorInfo(MonitorInfo* monitorInfo)
	{
		TFE_Settings_Window* windowSettings = TFE_Settings::getWindowSettings();

		s32 x = windowSettings->x, y = windowSettings->y;
		s32 displayIndex = getDisplayIndex(x, y);
		assert(displayIndex >= 0);

		getDisplayMonitorInfo(displayIndex, monitorInfo);
	}

	const WindowState& getWindowState()
	{
		return m_windowState;
	}

	void enableFullscreen(bool enable)
	{
		TFE_Settings_Window* windowSettings = TFE_Settings::getWindowSettings();
		windowSettings->fullscreen = enable;

		if (enable)
		{
			SDL_GetWindowPosition((SDL_Window*)m_window, &windowSettings->x, &windowSettings->y);
			s32 displayIndex = getDisplayIndex(windowSettings->x, windowSettings->y);
			if (displayIndex < 0)
			{
				displayIndex = 0;
				windowSettings->x = s_displayBounds[0].x;
				windowSettings->y = s_displayBounds[0].y + 32;
				windowSettings->baseWidth  = std::min(windowSettings->baseWidth,  (u32)s_displayBounds[0].w);
				windowSettings->baseHeight = std::min(windowSettings->baseHeight, (u32)s_displayBounds[0].h);
			}
			m_windowState.monitorWidth  = s_displayBounds[displayIndex].w;
			m_windowState.monitorHeight = s_displayBounds[displayIndex].h;

			m_windowState.flags |= WINFLAG_FULLSCREEN;

			SDL_RestoreWindow((SDL_Window*)m_window);
			SDL_SetWindowResizable((SDL_Window*)m_window, SDL_FALSE);
			SDL_SetWindowBordered((SDL_Window*)m_window, SDL_FALSE);

			SDL_SetWindowSize((SDL_Window*)m_window, m_windowState.monitorWidth, m_windowState.monitorHeight);
			SDL_SetWindowPosition((SDL_Window*)m_window, s_displayBounds[displayIndex].x, s_displayBounds[displayIndex].y);
		#ifndef _WIN32
			SDL_SetWindowFullscreen((SDL_Window*)m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
		#endif

			m_windowState.width  = m_windowState.monitorWidth;
			m_windowState.height = m_windowState.monitorHeight;
		}
		else
		{
			m_windowState.flags &= ~WINFLAG_FULLSCREEN;

		#ifndef _WIN32
			SDL_SetWindowFullscreen((SDL_Window*)m_window, 0);
		#endif
			SDL_RestoreWindow((SDL_Window*)m_window);
			SDL_SetWindowResizable((SDL_Window*)m_window, SDL_TRUE);
			SDL_SetWindowBordered((SDL_Window*)m_window, SDL_TRUE);

			SDL_SetWindowSize((SDL_Window*)m_window, m_windowState.baseWindowWidth, m_windowState.baseWindowHeight);
			SDL_SetWindowPosition((SDL_Window*)m_window, windowSettings->x, windowSettings->y);

			m_windowState.width  = m_windowState.baseWindowWidth;
			m_windowState.height = m_windowState.baseWindowHeight;
		}

		glViewport(0, 0, m_windowState.width, m_windowState.height);
		setupPostEffectChain(!s_useRenderTarget, s_bloomEnable);
		s_screenCapture->resize(m_windowState.width, m_windowState.height);
	}

	void clearWindow()
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void getDisplayInfo(DisplayInfo* displayInfo)
	{
		assert(displayInfo);

#if defined(ENABLE_VR)
		if (TFE_Settings::getTempSettings()->vr)
		{
			displayInfo->width = vr::GetRenderTargetSize().x;
			displayInfo->height = vr::GetRenderTargetSize().y;
			displayInfo->refreshRate = 70.0f;
		}
		else
#endif
		{
			displayInfo->width = m_windowState.width;
			displayInfo->height = m_windowState.height;
			displayInfo->refreshRate = (m_windowState.flags & WINFLAG_VSYNC) != 0 ? m_windowState.refreshRate : 0.0f;
		}
	}

	bool recreateDisplay(bool setupPostFx)
	{
		const bool inVr = TFE_Settings::getTempSettings()->vr;

		if (s_virtualDisplay)
		{
			delete s_virtualDisplay;
		}
		if (s_virtualRenderTarget)
		{
			delete s_virtualRenderTarget;
		}
		// Sync issue?
		if (s_virtualRenderTexture)
		{
			delete s_virtualRenderTexture;
		}
		if (s_virtualRenderDepthTexture)
		{
			delete s_virtualRenderDepthTexture;
		}
		if (s_materialRenderTexture)
		{
			delete s_materialRenderTexture;
		}
		s_virtualDisplay = nullptr;
		s_virtualRenderTarget = nullptr;
		s_virtualRenderTexture = nullptr;
		s_virtualRenderDepthTexture = nullptr;
		s_materialRenderTexture = nullptr;

		bool result = false;
		if (s_useRenderTarget)
		{
#if defined(ENABLE_VR)
			if (inVr)
			{
				const bool useMultiview = TFE_Settings::getTempSettings()->vrMultiview;
				const Vec2ui& renderTargetSize = vr::GetRenderTargetSize();
				s_virtualWidth = renderTargetSize.x;
				s_virtualHeight = renderTargetSize.y;

				s_virtualRenderTarget = new RenderTarget();
				s_virtualRenderTexture = new TextureGpu();
				s_virtualRenderDepthTexture = new TextureGpu();

				s_materialRenderTexture = s_bloomEnable ? new TextureGpu{} : nullptr;
				// TODO: match VR buffers formats
				if (useMultiview)
				{
					s_virtualRenderTexture->createArray(s_virtualWidth, s_virtualHeight, 2, TEX_RGBA8);
					s_virtualRenderDepthTexture->createArray(s_virtualWidth, s_virtualHeight, 2, TEX_DEPTH24_STENCIL8);
					if (s_materialRenderTexture)
					{
						s_materialRenderTexture->createArray(s_virtualWidth, s_virtualHeight, 2, TEX_RGBA8);
					}
				}
				else
				{
					s_virtualRenderTexture->create(s_virtualWidth, s_virtualHeight, TEX_RGBA8);
					s_virtualRenderDepthTexture->create(s_virtualWidth, s_virtualHeight, TEX_DEPTH24_STENCIL8);
					if (s_materialRenderTexture)
					{
						s_materialRenderTexture->create(s_virtualWidth, s_virtualHeight, TEX_RGBA8);
					}
				}

				if (s_bloomEnable) // Output to two textures.
				{
					TextureGpu* textures[] = { s_virtualRenderTexture, s_materialRenderTexture };
					s_virtualRenderTarget->create(2, textures, s_virtualRenderDepthTexture, useMultiview);
				}
				else
				{
					s_virtualRenderTarget->create(1, &s_virtualRenderTexture, s_virtualRenderDepthTexture, useMultiview);
				}
				result = true;
			}
			else
#endif
			{
				s_virtualRenderTarget = new RenderTarget();
				s_virtualRenderTexture = new TextureGpu();
				result = s_virtualRenderTexture->create(s_virtualWidth, s_virtualHeight);

				if (s_bloomEnable) // Output to two textures.
				{
					s_materialRenderTexture = new TextureGpu();
					result &= s_materialRenderTexture->create(s_virtualWidth, s_virtualHeight);

					TextureGpu* textures[] = { s_virtualRenderTexture, s_materialRenderTexture };
					result &= s_virtualRenderTarget->create(2, textures, true);
				}
				else
				{
					result &= s_virtualRenderTarget->create(1, &s_virtualRenderTexture, true);
				}
			}

			// The renderer will handle this instead.
			s_postEffectBlit->disableFeatures(BLIT_GPU_COLOR_CONVERSION);
			if (s_bloomEnable)
			{
				s_postEffectBlit->enableFeatures(BLIT_BLOOM);
			}
			else
			{
				s_postEffectBlit->disableFeatures(BLIT_BLOOM);
			}
			if (setupPostFx) { setupPostEffectChain(false, s_bloomEnable); }
		}
		else
		{
			s_virtualDisplay = new DynamicTexture();
			if (s_gpuColorConvert)
			{
				s_postEffectBlit->enableFeatures(BLIT_GPU_COLOR_CONVERSION);
			}
			else
			{
				s_postEffectBlit->disableFeatures(BLIT_GPU_COLOR_CONVERSION);
			}
			s_postEffectBlit->disableFeatures(BLIT_BLOOM);
			if (setupPostFx) { setupPostEffectChain(true, false); }
			result = s_virtualDisplay->create(s_virtualWidth, s_virtualHeight, s_asyncFrameBuffer ? 2 : 1, s_gpuColorConvert ? DTEX_R8 : DTEX_RGBA8);
		}
		return result;
	}

	// New version of the function.
	bool createVirtualDisplay(const VirtualDisplayInfo& vdispInfo)
	{
		const TFE_Settings_Graphics* graphicsSettings = TFE_Settings::getGraphicsSettings();
		s_virtualWidth = vdispInfo.width;
		s_virtualHeight = vdispInfo.height;
		s_virtualWidthUi = vdispInfo.widthUi;
		s_virtualWidth3d = vdispInfo.width3d;
		s_displayMode = vdispInfo.mode;
		s_widescreen = (vdispInfo.flags & VDISP_WIDESCREEN) != 0;
		s_asyncFrameBuffer = (vdispInfo.flags & VDISP_ASYNC_FRAMEBUFFER) != 0;
		s_gpuColorConvert = (vdispInfo.flags & VDISP_GPU_COLOR_CONVERT) != 0;
		s_useRenderTarget = (vdispInfo.flags & VDISP_RENDER_TARGET) != 0;
		s_bloomEnable = graphicsSettings->bloomEnabled && s_useRenderTarget;

		return recreateDisplay(true);
	}

	u32 getVirtualDisplayWidth2D()
	{
		return s_virtualWidthUi;
	}

	u32 getVirtualDisplayWidth3D()
	{
		return s_virtualWidth3d;
	}

	u32 getVirtualDisplayHeight()
	{
		return s_virtualHeight;
	}

	u32 getVirtualDisplayOffset2D()
	{
		if (s_virtualWidth <= s_virtualWidthUi) { return 0; }
		return (s_virtualWidth - s_virtualWidthUi) >> 1;
	}

	u32 getVirtualDisplayOffset3D()
	{
		if (s_virtualWidth <= s_virtualWidth3d) { return 0; }
		return (s_virtualWidth - s_virtualWidth3d) >> 1;
	}

	void* getVirtualDisplayGpuPtr()
	{
		return (void*)(iptr)s_virtualDisplay->getTexture()->getHandle();
	}

	bool getWidescreen()
	{
		return s_widescreen;
	}

	bool getFrameBufferAsync()
	{
		return s_asyncFrameBuffer;
	}

	bool getGPUColorConvert()
	{
		return s_gpuColorConvert;
	}

	void updateVirtualDisplay(const void* buffer, size_t size)
	{
		TFE_ZONE("Update Virtual Display");
		if (s_virtualDisplay)
		{
			s_virtualDisplay->update(buffer, size);
		}
	}

	void bindVirtualDisplay()
	{
		if (s_virtualRenderTarget)
		{
			s_virtualRenderTarget->bind();
		}
	}

	void clearVirtualDisplay(f32* color, bool clearColor)
	{
		if (s_virtualDisplay)
		{
			// TODO
		}
		else if (s_virtualRenderTarget)
		{
			s_virtualRenderTarget->clear(color, 1.0f, 0, clearColor);
		}
	}

	void copyToVirtualDisplay(RenderTargetHandle src)
	{
		RenderTarget::copy(s_virtualRenderTarget, (RenderTarget*)src);
	}
		
	void copyBackbufferToRenderTarget(RenderTargetHandle dst)
	{
		s_copyTarget = (RenderTarget*)dst;
	}

	void setPalette(const u32* palette)
	{
		if (palette && getGPUColorConvert())
		{
			TFE_ZONE("Update Palette");
			s_palette->update(palette, 256 * sizeof(u32));
		}
		memcpy(s_paletteCpu, palette, 256 * sizeof(u32));
	}

	const u32* getPalette()
	{
		return s_paletteCpu;
	}

	const TextureGpu* getPaletteTexture()
	{
		return s_palette->getTexture();
	}

	void setColorCorrection(bool enabled, const ColorCorrection* color/* = nullptr*/, bool bloomChanged/* = false*/)
	{
		if (bloomChanged)
		{
			TFE_Settings_Graphics* graphicsSettings = TFE_Settings::getGraphicsSettings();
			s_bloomEnable = graphicsSettings->bloomEnabled && s_useRenderTarget;
			recreateDisplay(false);
		}

		if (s_postEffectBlit->featureEnabled(BLIT_GPU_COLOR_CORRECTION) != enabled || bloomChanged)
		{
			if (enabled) { s_postEffectBlit->enableFeatures(BLIT_GPU_COLOR_CORRECTION); }
			else { s_postEffectBlit->disableFeatures(BLIT_GPU_COLOR_CORRECTION); }

			setupPostEffectChain(!s_useRenderTarget, s_bloomEnable);
		}

		if (color)
		{
			s_postEffectBlit->setColorCorrectionParameters(color);
		}
	}

	void drawVirtualDisplay()
	{
		TFE_ZONE("Draw Virtual Display");
		if (!s_virtualDisplay && !s_virtualRenderTarget) { return; }

		// Only clear if (1) s_virtualDisplay == null or (2) s_displayMode != DMODE_STRETCH
		if (s_displayMode != DMODE_STRETCH)
		{
			//glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			TFE_ASSERT_GL;
		}
		TFE_PostProcess::execute();
	}

	// GPU commands
	// core gpu functionality for UI and editor.
	// Render target.
	RenderTargetHandle createRenderTarget(u32 width, u32 height, bool hasDepthBuffer)
	{
		RenderTarget* newTarget = new RenderTarget();
		TextureGpu* texture = new TextureGpu();
		if (TFE_Settings::getTempSettings()->vr && TFE_Settings::getTempSettings()->vrMultiview)
		{
			texture->createArray(width, height, 2, TexFormat::TEX_RGBA8/*TODO: from vr*/);
			newTarget->create(1, &texture, nullptr, true);
		}
		else
		{
			texture->create(width, height);
			newTarget->create(1, &texture, hasDepthBuffer);
		}

		return RenderTargetHandle(newTarget);
	}

	void freeRenderTarget(RenderTargetHandle handle)
	{
		if (!handle) { return; }

		RenderTarget* renderTarget = (RenderTarget*)handle;
		delete renderTarget->getTexture();
		delete renderTarget;
	}

	void bindRenderTarget(RenderTargetHandle handle)
	{
		RenderTarget* renderTarget = (RenderTarget*)handle;
		renderTarget->bind();

		const TextureGpu* texture = renderTarget->getTexture();
		s_rtWidth = texture->getWidth();
		s_rtHeight = texture->getHeight();
	}

	void clearRenderTarget(RenderTargetHandle handle, const f32* clearColor, f32 clearDepth)
	{
		RenderTarget* renderTarget = (RenderTarget*)handle;
		renderTarget->clear(clearColor, clearDepth);
		setClearColor(s_clearColor);
	}

	void clearRenderTargetDepth(RenderTargetHandle handle, f32 clearDepth)
	{
		RenderTarget* renderTarget = (RenderTarget*)handle;
		renderTarget->clearDepth(clearDepth);
	}

	void copyRenderTarget(RenderTargetHandle dst, RenderTargetHandle src)
	{
		if (!dst || !src) { return; }
		RenderTarget::copy((RenderTarget*)dst, (RenderTarget*)src);
	}

	void unbindRenderTarget()
	{
#if defined(ENABLE_VR)
		if (TFE_Settings::getTempSettings()->vr)
		{
			RenderTarget& renderTarget = vr::GetRenderTarget(vr::Side::Left);
			renderTarget.bind();

			if (s_copyTarget)
			{
				RenderTarget::copy(s_copyTarget, &renderTarget);
				s_copyTarget = nullptr;
			}
		}
		else
#endif
		{
			RenderTarget::unbind();
			glViewport(0, 0, m_windowState.width, m_windowState.height);
			TFE_ASSERT_GL;

			if (s_copyTarget)
			{
				RenderTarget::copyBackbufferToTarget(s_copyTarget);
				s_copyTarget = nullptr;
			}
		}
	}

	void invalidateRenderTarget()
	{
#if defined(ENABLE_VR)
		if (TFE_Settings::getTempSettings()->vr)
		{
			RenderTarget& renderTarget = vr::GetRenderTarget(vr::Side::Left);
			renderTarget.invalidate();
		}
#endif
	}

	void setViewport(s32 x, s32 y, s32 w, s32 h)
	{
		glViewport(x, y, w, h);
		TFE_ASSERT_GL;
	}

	void setScissorRect(bool enable, s32 x, s32 y, s32 w, s32 h)
	{
		if (enable)
		{
			glScissor(x, y, w, h);
			glEnable(GL_SCISSOR_TEST);
		}
		else
		{
			glDisable(GL_SCISSOR_TEST);
		}
		TFE_ASSERT_GL;
	}

	const TextureGpu* getRenderTargetTexture(RenderTargetHandle rtHandle)
	{
		RenderTarget* renderTarget = (RenderTarget*)rtHandle;
		return renderTarget->getTexture();
	}

	void getRenderTargetDim(RenderTargetHandle rtHandle, u32* width, u32* height)
	{
		RenderTarget* renderTarget = (RenderTarget*)rtHandle;
		const TextureGpu* texture = renderTarget->getTexture();
		*width = texture->getWidth();
		*height = texture->getHeight();
	}

	TextureGpu* createTexture(u32 width, u32 height, TexFormat format)
	{
		TextureGpu* texture = new TextureGpu();
		texture->create(width, height, format);
		return texture;
	}

	TextureGpu* createTextureArray(u32 width, u32 height, u32 layers, u32 channels, u32 mipCount)
	{
		TextureGpu* texture = new TextureGpu();
		texture->createArray(width, height, layers, channels, mipCount);
		return texture;
	}

	// Create a GPU version of a texture, assumes RGBA8 and returns a GPU handle.
	TextureGpu* createTexture(u32 width, u32 height, const u32* data, MagFilter magFilter)
	{
		TextureGpu* texture = new TextureGpu();
		texture->createWithData(width, height, data, magFilter);
		return texture;
	}

	void freeTexture(TextureGpu* texture)
	{
		if (!texture) { return; }
		delete texture;
	}

	void getTextureDim(TextureGpu* texture, u32* width, u32* height)
	{
		*width = texture->getWidth();
		*height = texture->getHeight();
	}

	void* getGpuPtr(const TextureGpu* texture)
	{
		return (void*)(iptr)texture->getHandle();
	}

	void bindNativeTexture(void* texture)
	{
		glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)texture);
		TFE_ASSERT_GL;
	}

	void drawIndexedTriangles(u32 triCount, u32 indexStride, u32 indexStart)
	{
		GLint buffer;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &buffer);
		TFE_ASSERT_GL;
		if (buffer == 0)
		{
			std::ignore = 5;
		}

		glDrawElements(GL_TRIANGLES, triCount * 3, indexStride == 4 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT, (void*)(iptr)(indexStart * indexStride));
		TFE_ASSERT_GL;
	}

	void drawLines(u32 lineCount)
	{
		glDrawArrays(GL_LINES, 0, lineCount * 2);
		TFE_ASSERT_GL;
	}

	static u32 s_bloomBufferCount = 0;
	static RenderTarget* s_bloomTargets[16] = { 0 };
	static TextureGpu* s_bloomTextures[16] = { 0 };

	void setupBloomStages()
	{
		// Free existing buffers...
		for (u32 i = 0; i < s_bloomBufferCount; i++)
		{
			delete s_bloomTargets[i];
			delete s_bloomTextures[i];
			s_bloomTargets[i] = nullptr;
			s_bloomTextures[i] = nullptr;
		}
		s_bloomBufferCount = 0;

		// Create new textures and stages.
		const TextureGpu* baseTex = s_virtualRenderTarget->getTexture();
		u32 width  = baseTex->getWidth()/2;
		u32 height = baseTex->getHeight()/2;

		// Threshold.
		s32 index = s_bloomBufferCount;
		s_bloomBufferCount++;

		const TexFormat texFormat = TFE_Settings::getGraphicsSettings()->bloomUseHalfFloatFormat ? TexFormat::TEX_R11F_G11F_B10F : TexFormat::TEX_RGBAF16;

		if (TFE_Settings::getTempSettings()->vrMultiview)
		{
			s_bloomTextures[index] = new TextureGpu();
			s_bloomTextures[index]->createArray(width, height, 2, texFormat, false, MAG_FILTER_LINEAR);
			s_bloomTargets[index] = new RenderTarget();
			s_bloomTargets[index]->create(1, &s_bloomTextures[index], nullptr, true);
		}
		else
		{
			s_bloomTextures[index] = new TextureGpu();
			s_bloomTextures[index]->create(width, height, texFormat, false, MAG_FILTER_LINEAR);
			s_bloomTargets[index] = new RenderTarget();
			s_bloomTargets[index]->create(1, &s_bloomTextures[index], false);
		}

		const PostEffectInput bloomTresholdInputs[] =
		{
			{ PTYPE_TEXTURE, (void*)s_virtualRenderTarget->getTexture(0) },
			{ PTYPE_TEXTURE, (void*)s_virtualRenderTarget->getTexture(1) },
		};
		TFE_PostProcess::appendEffect(s_bloomTheshold, TFE_ARRAYSIZE(bloomTresholdInputs), bloomTresholdInputs, s_bloomTargets[index], 0, 0, width, height, true);

		// Downscale.
		while (s_bloomBufferCount < 8 && width > 8 && height > 8)
		{
			s32 index = s_bloomBufferCount;
			s_bloomBufferCount++;

			width  >>= 1;
			height >>= 1;
			if (TFE_Settings::getTempSettings()->vrMultiview)
			{
				s_bloomTextures[index] = new TextureGpu();
				s_bloomTextures[index]->createArray(width, height, 2, texFormat, false, MAG_FILTER_LINEAR);
				s_bloomTargets[index] = new RenderTarget();
				s_bloomTargets[index]->create(1, &s_bloomTextures[index], nullptr, true);
			}
			else
			{
				s_bloomTextures[index] = new TextureGpu();
				s_bloomTextures[index]->create(width, height, texFormat, false, MAG_FILTER_LINEAR);
				s_bloomTargets[index] = new RenderTarget();
				s_bloomTargets[index]->create(1, &s_bloomTextures[index], false);
			}

			const PostEffectInput bloomDownsampleInputs[] =
			{
				{ PTYPE_TEXTURE, (void*)s_bloomTextures[index - 1] },
			};
			TFE_PostProcess::appendEffect(s_bloomDownsample, TFE_ARRAYSIZE(bloomDownsampleInputs), bloomDownsampleInputs, s_bloomTargets[index], 0, 0, width, height);
		}

		// Upscale and Merge.
		s32 end = s_bloomBufferCount - 1;
		for (s32 i = end; i > 0; i--)
		{
			width  = s_bloomTextures[i - 1]->getWidth();
			height = s_bloomTextures[i - 1]->getHeight();

			s32 index = s_bloomBufferCount;
			s_bloomBufferCount++;

			if (TFE_Settings::getTempSettings()->vrMultiview)
			{
				s_bloomTextures[index] = new TextureGpu();
				s_bloomTextures[index]->createArray(width, height, 2, texFormat, false, MAG_FILTER_LINEAR);
				s_bloomTargets[index] = new RenderTarget();
				s_bloomTargets[index]->create(1, &s_bloomTextures[index], nullptr, true);
			}
			else
			{
				s_bloomTextures[index] = new TextureGpu();
				s_bloomTextures[index]->create(width, height, texFormat, false, MAG_FILTER_LINEAR);
				s_bloomTargets[index] = new RenderTarget();
				s_bloomTargets[index]->create(1, &s_bloomTextures[index], false);
			}

			const PostEffectInput bloomMergeInputs[] =
			{
				{ PTYPE_TEXTURE, (void*)s_bloomTextures[i - 1] },
				{ PTYPE_TEXTURE, s_bloomTextures[index - 1] },
			};
			TFE_PostProcess::appendEffect(s_bloomMerge, TFE_ARRAYSIZE(bloomMergeInputs), bloomMergeInputs, s_bloomTargets[index], 0, 0, width, height);
		}

		// Final result is in s_bloomTargets[s_bloomBufferCount-1]
	}

	// A quick way of toggling the bloom, but just for the final blit.
	void bloomPostEnable(bool enable)
	{
		if (!s_bloomEnable) { return; }
		if (enable) { s_postEffectBlit->enableFeatures(BLIT_BLOOM); }
		else        { s_postEffectBlit->disableFeatures(BLIT_BLOOM); }
	}

	// Setup the Post effect chain based on current settings.
	// TODO: Move out of render backend since this should be independent of the backend.
	void setupPostEffectChain(bool useDynamicTexture, bool useBloom)
	{
		s32 x = 0, y = 0;
		s32 w = m_windowState.width;
		s32 h = m_windowState.height;
		f32 aspect = f32(w) / f32(h);

#if defined(ENABLE_VR)
		if (TFE_Settings::getTempSettings()->vr)
		{
			w = vr::GetRenderTargetSize().x;
			h = vr::GetRenderTargetSize().y;

			// Disable widescreen.
			s_widescreen = false;
			TFE_Settings::getGraphicsSettings()->widescreen = false;
		}
		else 
#endif
		if (s_displayMode == DMODE_ASPECT_CORRECT && aspect > c_tallScreenThreshold && !s_widescreen)
		{
			// Calculate width based on height.
			w = 4 * m_windowState.height / 3;
			h = m_windowState.height;

			// pillarbox
			x = std::max(0, ((s32)m_windowState.width - w) / 2);
		}
		else if (s_displayMode == DMODE_ASPECT_CORRECT && (!s_widescreen || aspect <= c_tallScreenThreshold))
		{
			// Disable widescreen.
			s_widescreen = false;
			TFE_Settings::getGraphicsSettings()->widescreen = false;

			// Calculate height based on width.
			h = 3 * m_windowState.width / 4;
			w = m_windowState.width;

			// letterbox
			y = std::max(0, ((s32)m_windowState.height - h) / 2);
		}
		TFE_PostProcess::clearEffectStack();

		if (useDynamicTexture)
		{
			const PostEffectInput blitInputs[] =
			{
				{ PTYPE_DYNAMIC_TEX, s_virtualDisplay },
				{ PTYPE_DYNAMIC_TEX, s_palette }
			};
			TFE_PostProcess::appendEffect(s_postEffectBlit, TFE_ARRAYSIZE(blitInputs), blitInputs, nullptr, x, y, w, h);
		}
		else if (useBloom)
		{
			setupBloomStages();

			const PostEffectInput blitInputs[] =
			{
				{ PTYPE_TEXTURE, (void*)s_virtualRenderTarget->getTexture(0) },
				{ PTYPE_TEXTURE, (void*)s_bloomTextures[s_bloomBufferCount-1] },
				{ PTYPE_DYNAMIC_TEX, s_palette }
			};
			TFE_PostProcess::appendEffect(s_postEffectBlit, TFE_ARRAYSIZE(blitInputs), blitInputs, nullptr, x, y, w, h);
		}
		else
		{
			const PostEffectInput blitInputs[] =
			{
				{ PTYPE_TEXTURE, (void*)s_virtualRenderTarget->getTexture() },
				{ PTYPE_DYNAMIC_TEX, s_palette }
			};
			TFE_PostProcess::appendEffect(s_postEffectBlit, TFE_ARRAYSIZE(blitInputs), blitInputs, nullptr, x, y, w, h);
		}
	}

	void resetBloom()
	{
		bool optimized = TFE_Settings::getGraphicsSettings()->bloomUseOptimizedShaders;

		delete s_bloomTheshold;
		s_bloomTheshold = new BloomThreshold{ optimized };
		s_bloomTheshold->init();

		delete s_bloomDownsample;
		s_bloomDownsample = new BloomDownsample{ optimized };
		s_bloomDownsample->init();

		delete s_bloomMerge;
		s_bloomMerge = new BloomMerge{ optimized /*TODO: ignored inside*/ };
		s_bloomMerge->init();

		setupPostEffectChain(false, TFE_Settings::getGraphicsSettings()->bloomEnabled);
	}

	void updateVRCamera()
	{
#if defined(ENABLE_VR)
		if (!TFE_Settings::getTempSettings()->vr)
		{
			return;
		}

		auto UpdateCameraProj = [](Mat4 cameraProj[], bool yUp) {
			cameraProj[0] = vr::GetEyeProj(vr::Side::Left, yUp);
			cameraProj[1] = vr::GetEyeProj(vr::Side::Right, yUp);
		};

		auto UpdateCameraMtx = [](Mat3 cameraMtx[], Vec3f cameraPos[], bool yUp) {
			Mat4 transformation[2] = {
				yUp ? vr::GetEyePose(vr::Side::Left).mTransformation : vr::GetEyePose(vr::Side::Left).mTransformationYDown,
				yUp ? vr::GetEyePose(vr::Side::Right).mTransformation : vr::GetEyePose(vr::Side::Right).mTransformationYDown };
			transformation[0].m3 = { 0.0f, 0.0f, 0.0f, 1.0f };
			transformation[1].m3 = { 0.0f, 0.0f, 0.0f, 1.0f };

			//Vec3f pos[2] = { TFE_Math::getVec3(transformation[0].m3), TFE_Math::getVec3(transformation[1].m3) };
			//const float eyeDist = TFE_Math::distance(&pos[0], &pos[1]);
			const float eyeDist = vr::GetEyesDistance() * TFE_Settings::getVrSettings()->playerScale;

			Mat4 view4 = TFE_Math::buildMatrix4(TFE_Jedi::s_cameraMtx, TFE_Jedi::s_cameraPos);

			Mat4 cameraMtx4[2] = {
				TFE_Math::mulMatrix4(TFE_Math::transpose4(transformation[0]), view4),
				TFE_Math::mulMatrix4(TFE_Math::transpose4(transformation[1]), view4)
			};
			cameraMtx[0] = TFE_Math::getMatrix3(cameraMtx4[0]);
			cameraMtx[1] = TFE_Math::getMatrix3(cameraMtx4[1]);
			Vec3f right0_ = TFE_Math::getVec3(cameraMtx4[0].m0);
			Vec3f right1_ = TFE_Math::getVec3(cameraMtx4[1].m0);
			Vec3f right0 = (0.5f * eyeDist) * TFE_Math::normalize(right0_);
			Vec3f left0 = -right0;
			Vec3f right1 = (0.5f * eyeDist) * TFE_Math::normalize(right0_);
			Vec3f left1 = -right1;
			//Vec3f s_cameraPos_[2] = { TFE_Math::add(TFE_Math::getTranslation(s_cameraMtxVr[0]), pos[0]), TFE_Math::add(TFE_Math::getTranslation(s_cameraMtxVr[1]), pos[1]) };
			//Vec3f s_cameraPos_[2] = { TFE_Math::add(TFE_Math::getTranslation(s_cameraMtxVr[0]), s_cameraPos), TFE_Math::add(TFE_Math::getTranslation(s_cameraMtxVr[1]), s_cameraPos) };
			cameraPos[0] = TFE_Math::getTranslation(cameraMtx4[0]) + left0;
			cameraPos[1] = TFE_Math::getTranslation(cameraMtx4[1]) + right0;
		};

		UpdateCameraProj(s_cameraProjVR, true);
		UpdateCameraProj(s_cameraProjVR_YDown, false);

		UpdateCameraMtx(s_cameraMtxVR, s_cameraPosVR, true);
		UpdateCameraMtx(s_cameraMtxVR_YDown, s_cameraPosVR_YDown, false);
#endif
	}
}  // namespace
