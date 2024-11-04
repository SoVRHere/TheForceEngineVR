#pragma once

#include <TFE_System/math.h>
#include <TFE_RenderBackend/Win32OpenGL/renderTarget.h>
#include <array>

namespace vr
{
	enum Side
	{
		Left = 0,
		Right,
		Count
	};

	struct Pose
	{
		Mat4	mTransformation{ TFE_Math::getIdentityMatrix4() };
		Mat4	mTransformationYDown{ TFE_Math::getIdentityMatrix4() };
		bool	mIsValid{ false };
	};

	enum ControllerButtons
	{
		A = 1 << 0, // X
		B = 1 << 1, // Y
		Menu = 1 << 2,
		Thumb = 1 << 3,
		Shoulder = 1 << 4,
	};

	enum class UpdateStatus
	{
		Ok,
		ShouldQuit,
		ShouldDestroy,
		ShouldNotRender,
		NotVisible
	};

	struct ControllerState
	{
		uint32_t	mControllerButtons{ 0 };
		float		mHandTrigger{ 0.0f };
		float		mIndexTrigger{ 0.0f };
		Vec2f		mThumbStick{ 0.0f, 0.0f };
		Vec2f		mTrackpad{ 0.0f, 0.0f };
	};

	//struct HapticVibration
	//{
	//	float mDuration;	// seconds, < 0.0f = minimal
	//	float mFrequency;	// Hz, 0.0f = unspecified
	//	float mAmplitude;	// [0.0f, 1.0f]
	//};

	bool Initialize(bool useMultiview);
	bool IsInitialized();
	void Shutdown();
	const char* GetRuntimeInfo();

	const Vec2ui& GetRenderTargetSize();
	RenderTarget& GetRenderTarget(Side eye);

	UpdateStatus UpdateFrame(float cameraNear, float cameraFar);
	void UpdateView(Side eye);
	void Commit(Side eye);
	void SubmitFrame();

	void Reset();

	const Mat4& GetEyeProj(Side eye, bool yUp);
	const Pose& GetEyePose(Side eye);
	const Vec2f& GetFov();
	float GetEyesDistance();
	const Mat4& GetUnitedProj(bool yUp);
	const std::array<Vec3f, 8>& GetUnitedFrustum();

	//const Pose& GetHandPose(Side hand);
	//const ControllerState& GetControllerState(Side hand);
}
