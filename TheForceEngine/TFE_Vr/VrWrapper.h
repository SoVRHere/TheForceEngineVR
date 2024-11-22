#pragma once

#if defined(_WIN32)
	#if defined(VR_BUILD_LIB)  // when building the lib
		#define VR_WRAPPER_API __declspec(dllexport)
	#else  // when using the DLL
		#define VR_WRAPPER_API __declspec(dllimport)
	#endif
#else
	#error "handle non win32 platform here"
#endif

#include <cstdint>

namespace vrw
{
	struct Vec2ui
	{
		uint32_t x;
		uint32_t y;
	};

	struct Vec2f
	{
		float x;
		float y;
	};

	struct Vec3f
	{
		float x;
		float y;
		float z;
	};

	struct Mat4f
	{
		float m[16];
	};

	struct Texture
	{
		Texture() = default;
		Texture(uint32_t glHandle) : glHandle{ glHandle } {}
		Texture(void* vkHandle) : vkHandle{ vkHandle } {}

		union
		{
			uint32_t glHandle; // GLuint
			void* vkHandle; // VkImage
		};
	};

	class VR_WRAPPER_API VrWrapper
	{
	public:
		enum class VrApi : uint32_t
		{
			OpenXR,
			OpenVR
		};

		enum class GfxApi : uint32_t
		{
			OpenGL,
			Vulkan
		};

		enum class MessageSeverity : uint32_t
		{
			Debug,
			Info,
			Warning,
			Error,
			Critical
		};

		using MessageCallback = void(MessageSeverity messageSeverity, const char* message);

		enum Side : uint32_t
		{
			Left = 0,
			Right,
			Count
		};

		enum class Feature : uint32_t
		{
			EyeTracking = 1 << 0,
			Passthrough = 1 << 1
		};

		struct Options
		{
			VrApi				vrApi;
			GfxApi				gfxApi;
			bool				multiview;
			uint64_t			featuresToInitialize;
			MessageCallback*	messageCallback;
			MessageSeverity		messageLevel;
		};

		enum class UpdateStatus : uint32_t
		{
			Ok,
			ShouldQuit,
			ShouldDestroy,
			ShouldNotRender,
			NotVisible
		};

		struct Pose
		{
			Mat4f	mTransformation;
			Mat4f	mTransformationYDown;
			bool	mIsValid{ false };
		};

		enum class ControllerButton : uint32_t
		{
			A = 1 << 0, // X
			B = 1 << 1, // Y
			Menu = 1 << 2,
			Thumb = 1 << 3,
			Shoulder = 1 << 4,
		};

		struct ControllerState
		{
			uint32_t	mButtons{ 0 };
			float		mHandTrigger{ 0.0f };
			float		mIndexTrigger{ 0.0f };
			Vec2f		mThumbStick{ 0.0f, 0.0f };
			Vec2f		mTrackpad{ 0.0f, 0.0f };
		};

	public:
		static VrWrapper* Initialize(const Options& options);
		virtual ~VrWrapper();

		virtual bool IsInitialized() = 0;
		virtual bool IsFeatureSupported(Feature feature) = 0;
		virtual const char* GetRuntimeInfo() = 0;

		virtual const Vec2ui& GetSwapchainTextureSize() = 0;
		virtual uint32_t GetSwapchainLength() = 0;
		virtual uint32_t GetSwapchainTextures(const Texture** textures) = 0;
		virtual uint32_t GetSwapchainIndex() = 0;

		virtual UpdateStatus UpdateFrame(float cameraNear, float cameraFar) = 0;
		virtual void UpdateView(Side eye) = 0;
		virtual void Commit(Side eye) = 0;
		virtual void SubmitFrame() = 0;

		virtual const Mat4f& GetEyeProj(Side eye, bool yUp) = 0;
		virtual const Pose& GetEyePose(Side eye) = 0;
		virtual const Vec2f& GetFov() = 0;
		virtual float GetEyesDistance() = 0;
		virtual const Mat4f& GetUnitedProj(bool yUp) = 0;
		virtual const Vec3f* GetUnitedFrustum() = 0;

		virtual const Pose& GetPointerPose(Side side) = 0;
		virtual const Pose& GetControllerPose(Side side) = 0;
		virtual const ControllerState& GetControllerState(Side side) = 0;
	};
}
