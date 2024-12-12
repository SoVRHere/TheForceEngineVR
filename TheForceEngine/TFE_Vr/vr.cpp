#include "vr.h"
#include <TFE_System/system.h>
#include <TFE_Settings/settings.h>
#include <TFE_Input/input.h>
#include <TFE_DarkForces/GameUI/escapeMenu.h>
#include <TFE_DarkForces/GameUI/pda.h>
#include <TFE_FrontEndUI/frontEndUi.h>

#include "VrWrapper.h"

namespace vr
{
	vrw::VrWrapper* g_VrWrapper = nullptr;

	Vec2ui			 									mTargetSize{ 0, 0 };

	std::array<Mat4, Side::Count>						mProjection{ TFE_Math::getIdentityMatrix4(), TFE_Math::getIdentityMatrix4() };
	std::array<Mat4, Side::Count>						mProjectionYDown{ TFE_Math::getIdentityMatrix4(), TFE_Math::getIdentityMatrix4() };
	std::array<Pose, Side::Count>						mEyePose;
	Mat4												mUnitedProjection{ TFE_Math::getIdentityMatrix4() };
	Mat4												mUnitedProjectionDown{ TFE_Math::getIdentityMatrix4() };
	std::array<Vec3f, 8>								mUnitedFrustum;
	Vec2f												mFov{ 0.0f, 0.0f };
	float												mEyesDistance{ 0.0f };

	std::array<Pose, Side::Count>						mPointerPose;
	std::array<Pose, Side::Count>						mControllerPose;
	std::array<ControllerState, Side::Count>			mControllerState;

	std::array<std::vector<std::unique_ptr<RenderTarget>>, Side::Count>	mRenderTargets;
	uint32_t											mSwapchainIndex{ 0 };

	std::array<bool, CONTROLLER_BUTTON_COUNT>			mControllerButtonPressed;

	enum class ThumbSection
	{
		LeftThumbUp = 0,
		LeftThumbDown,
		LeftThumbLeft,
		LeftThumbRight,
		RightThumbUp,
		RightThumbDown,
		RightThumbLeft,
		RightThumbRight,
		Count
	};

	enum class ThumbSectionState
	{
		None,
		Down,
		Up,
		Pressed,
		Count
	};

	std::array<ThumbSectionState, (size_t)ThumbSection::Count>	mThumbSectionState;

	std::array<Pose, Side::Count>						mLastPointerPose;

	std::vector<SDL_Event>								mEmulatedEvents;
	uint32_t											mMouseButtonsPressed{ 0 };
	Vec2i												mPointerMousePos{ -10, -10 };
	Vec2i												mLastPointerMousePos = { -10, -10 };

	void UpdateView(Side eye)
	{
		if (!IsInitialized())
		{
			return;
		}

		g_VrWrapper->UpdateView((vrw::VrWrapper::Side)eye);
		mSwapchainIndex = g_VrWrapper->GetSwapchainIndex();
	}

	void Commit(Side eye)
	{
		if (!IsInitialized())
		{
			return;
		}

		g_VrWrapper->Commit((vrw::VrWrapper::Side)eye);
	}

	void SubmitFrame()
	{
		if (!IsInitialized())
		{
			return;
		}

		return g_VrWrapper->SubmitFrame();
	}

	void Reset()
	{
		// TODO: implement
	}

	const Mat4& GetEyeProj(Side eye, bool yUp)
	{
		return yUp ? mProjection[eye] : mProjectionYDown[eye];
	}

	const Pose& GetEyePose(Side eye)
	{
		return mEyePose[eye];
	}

	const Vec2f& GetFov()
	{
		return mFov;
	}

	float GetEyesDistance()
	{
		return mEyesDistance;
	}

	const Mat4& GetUnitedProj(bool yUp)
	{
		return yUp ? mUnitedProjection : mUnitedProjectionDown;
	}

	const std::array<Vec3f, 8>& GetUnitedFrustum()
	{
		return mUnitedFrustum;
	}

	const Pose& GetPointerPose(Side side)
	{
		return mPointerPose[side];
	}

	const Pose& GetControllerPose(Side side)
	{
		return mControllerPose[side];
	}

	const ControllerState& GetControllerState(Side side)
	{
		return mControllerState[side];
	}

	void MessageCallback(vrw::VrWrapper::MessageSeverity messageSeverity, const char* message)
	{
		switch (messageSeverity)
		{
		case vrw::VrWrapper::MessageSeverity::Critical:
		case vrw::VrWrapper::MessageSeverity::Error:
			TFE_ERROR("VR", "{}", message);
			break;
		case vrw::VrWrapper::MessageSeverity::Warning:
			TFE_WARN("VR", "{}", message);
			break;
		case vrw::VrWrapper::MessageSeverity::Debug:
		case vrw::VrWrapper::MessageSeverity::Info:
		default:
			TFE_INFO("VR", "{}", message);
			break;
		}
	}

	bool Initialize(bool useMultiview)
	{
		vrw::VrWrapper::Options options{
			vrw::VrWrapper::VrApi::OpenXR,
			vrw::VrWrapper::GfxApi::OpenGL,
			useMultiview,
			0,
			&MessageCallback,
#if defined(_DEBUG)
			vrw::VrWrapper::MessageSeverity::Debug
#else
			vrw::VrWrapper::MessageSeverity::Info
#endif
		};
		g_VrWrapper = vrw::VrWrapper::Initialize(options);
		if (g_VrWrapper && !g_VrWrapper->IsInitialized())
		{
			SafeDelete(g_VrWrapper); // something went wrong
		}

		if (g_VrWrapper)
		{
			// create render targets from swapchain images
			std::vector<const vrw::Texture*> textures;
			const uint32_t numTextures = g_VrWrapper->GetSwapchainTextures(nullptr);
			textures.resize(numTextures);
			g_VrWrapper->GetSwapchainTextures(textures.data());
			const uint32_t swapchainLength = g_VrWrapper->GetSwapchainLength();
			const vrw::Vec2ui size = g_VrWrapper->GetSwapchainTextureSize();
			mTargetSize = { size.x, size.y };
			const uint32_t width = mTargetSize.x;
			const uint32_t height = mTargetSize.y;

			size_t textureIndex = 0;
			const size_t bufferCount = useMultiview ? 1 : 2;
			for (size_t eye = 0; eye < bufferCount; eye++)
			{
				mRenderTargets[eye].reserve(swapchainLength);
				for (size_t i = 0; i < swapchainLength; i++, textureIndex++)
				{
					std::unique_ptr<TextureGpu> colorTexture = std::make_unique<TextureGpu>(textures[textureIndex]->glHandle); // color texture is owned by VR API
					std::unique_ptr<TextureGpu> depthTexture = std::make_unique<TextureGpu>();
					if (useMultiview)
					{
						colorTexture->createArray(width, height, 2, TEX_RGBA8); // TODO: use image.image (pixelFormat) format
						depthTexture->createArray(width, height, 2, TEX_DEPTH24_STENCIL8);
					}
					else
					{
						colorTexture->create(width, height, TEX_RGBA8); // TODO: use image.image (pixelFormat) format
						depthTexture->create(width, height, TEX_DEPTH24_STENCIL8);
					}

					auto renderTarget = std::make_unique<RenderTarget>();
					TextureGpu* colorTexture_ = colorTexture.release();
					renderTarget->create(1, &colorTexture_, depthTexture.release(), useMultiview);
					mRenderTargets[eye].push_back(std::move(renderTarget));
				}
			}

			// controllers
			mControllerButtonPressed.fill(false);
			mThumbSectionState.fill(ThumbSectionState::None);
		}

		return g_VrWrapper != nullptr;
	}

	bool IsInitialized()
	{
		return g_VrWrapper && g_VrWrapper->IsInitialized();
	}

	void Shutdown()
	{
		for (size_t eye = 0; eye < Side::Count; eye++)
		{
			mRenderTargets[eye].clear();
		}

		SafeDelete(g_VrWrapper);
	}

	const char* GetRuntimeInfo()
	{
		return g_VrWrapper ? g_VrWrapper->GetRuntimeInfo() : "not available";
	}

	const Vec2ui& GetRenderTargetSize()
	{
		return mTargetSize;
	}

	RenderTarget& GetRenderTarget(Side eye)
	{
		auto& renderTarget = mRenderTargets[eye][mSwapchainIndex];
		return *renderTarget;
	}

	Mat4 VrMat4ToTFEMat4(const vrw::Mat4f& mat)
	{
		Mat4 result;
		std::memcpy(result.m, mat.m, sizeof(result.m));
		return result;
	}

	Vec2f VrVec2fToTFEVec2f(const vrw::Vec2f& v)
	{
		return { v.x, v.y };
	}

	UpdateStatus UpdateFrame(float cameraNear, float cameraFar)
	{
		if (!IsInitialized())
		{
			return UpdateStatus::ShouldQuit;
		}

		const UpdateStatus status = (UpdateStatus)g_VrWrapper->UpdateFrame(cameraNear, cameraFar);
		if (status != UpdateStatus::ShouldQuit)
		{
			mProjection[Side::Left] = VrMat4ToTFEMat4(g_VrWrapper->GetEyeProj(vrw::VrWrapper::Side::Left, true));
			mProjection[Side::Right] = VrMat4ToTFEMat4(g_VrWrapper->GetEyeProj(vrw::VrWrapper::Side::Right, true));
			mProjectionYDown[Side::Left] = VrMat4ToTFEMat4(g_VrWrapper->GetEyeProj(vrw::VrWrapper::Side::Left, false));
			mProjectionYDown[Side::Right] = VrMat4ToTFEMat4(g_VrWrapper->GetEyeProj(vrw::VrWrapper::Side::Right, false));

			{
				const vrw::VrWrapper::Pose& eyePoseLeft = g_VrWrapper->GetEyePose(vrw::VrWrapper::Side::Left);
				const vrw::VrWrapper::Pose& eyePoseRight = g_VrWrapper->GetEyePose(vrw::VrWrapper::Side::Right);
				mEyePose[Side::Left] = { VrMat4ToTFEMat4(eyePoseLeft.mTransformation), VrMat4ToTFEMat4(eyePoseLeft.mTransformationYDown), eyePoseLeft.mIsValid };
				mEyePose[Side::Right] = { VrMat4ToTFEMat4(eyePoseRight.mTransformation), VrMat4ToTFEMat4(eyePoseRight.mTransformationYDown), eyePoseRight.mIsValid };
			}

			mUnitedProjection = VrMat4ToTFEMat4(g_VrWrapper->GetUnitedProj(true));
			mUnitedProjectionDown = VrMat4ToTFEMat4(g_VrWrapper->GetUnitedProj(false));

			const vrw::Vec3f* frustumPoints = g_VrWrapper->GetUnitedFrustum();
			for (size_t i = 0; i < 8; i++)
			{
				mUnitedFrustum[i] = { frustumPoints[i].x, frustumPoints[i].y, frustumPoints[i].z };
			}
			// make sure frustum is symmetrical, Quest 3 returns non-symmetrical frustum
			const float nearHeightHalf = (mUnitedFrustum[2].y - mUnitedFrustum[0].y) / 2.0f;
			mUnitedFrustum[0].y = -nearHeightHalf;
			mUnitedFrustum[1].y = -nearHeightHalf;
			mUnitedFrustum[2].y = nearHeightHalf;
			mUnitedFrustum[3].y = nearHeightHalf;
			const float farHeightHalf = (mUnitedFrustum[6].y - mUnitedFrustum[4].y) / 2.0f;
			mUnitedFrustum[4].y = -farHeightHalf;
			mUnitedFrustum[5].y = -farHeightHalf;
			mUnitedFrustum[6].y = farHeightHalf;
			mUnitedFrustum[7].y = farHeightHalf;

			mFov = { g_VrWrapper->GetFov().x, g_VrWrapper->GetFov().y };
			mEyesDistance = g_VrWrapper->GetEyesDistance();

			{
				const vrw::VrWrapper::Pose& pointerPoseLeft = g_VrWrapper->GetPointerPose(vrw::VrWrapper::Side::Left);
				const vrw::VrWrapper::Pose& pointerPoseRight = g_VrWrapper->GetPointerPose(vrw::VrWrapper::Side::Right);
				mPointerPose[Side::Left] = { VrMat4ToTFEMat4(pointerPoseLeft.mTransformation), VrMat4ToTFEMat4(pointerPoseLeft.mTransformationYDown), pointerPoseLeft.mIsValid };
				mPointerPose[Side::Right] = { VrMat4ToTFEMat4(pointerPoseRight.mTransformation), VrMat4ToTFEMat4(pointerPoseRight.mTransformationYDown), pointerPoseRight.mIsValid };
			}
			//TFE_INFO("VR", "Pointer pose left: {} {} {}", mPointerPose[Side::Left].mTransformation.data[12], mPointerPose[Side::Left].mTransformation.data[13], mPointerPose[Side::Left].mTransformation.data[14]);

			{
				const vrw::VrWrapper::Pose& controllerPoseLeft = g_VrWrapper->GetControllerPose(vrw::VrWrapper::Side::Left);
				const vrw::VrWrapper::Pose& controllerPoseRight = g_VrWrapper->GetControllerPose(vrw::VrWrapper::Side::Right);
				mControllerPose[Side::Left] = { VrMat4ToTFEMat4(controllerPoseLeft.mTransformation), VrMat4ToTFEMat4(controllerPoseLeft.mTransformationYDown), controllerPoseLeft.mIsValid };
				mControllerPose[Side::Right] = { VrMat4ToTFEMat4(controllerPoseRight.mTransformation), VrMat4ToTFEMat4(controllerPoseRight.mTransformationYDown), controllerPoseRight.mIsValid };

				const vrw::VrWrapper::ControllerState& csl = g_VrWrapper->GetControllerState(vrw::VrWrapper::Side::Left);
				const vrw::VrWrapper::ControllerState& csr = g_VrWrapper->GetControllerState(vrw::VrWrapper::Side::Right);
				mControllerState[Side::Left] = { csl.mButtons, csl.mHandTrigger, csl.mIndexTrigger, VrVec2fToTFEVec2f(csl.mThumbStick), VrVec2fToTFEVec2f(csl.mTrackpad) };
				mControllerState[Side::Right] = { csr.mButtons, csr.mHandTrigger, csr.mIndexTrigger, VrVec2fToTFEVec2f(csr.mThumbStick), VrVec2fToTFEVec2f(csr.mTrackpad) };
			}
		}
		else
		{
			std::ignore = 5;
		}

		return status;
	}

	Vec2i GetMousePosByPointer(const TFE_Settings_Vr::ScreenToVr* screenToVr)
	{
		Vec2i pointerMousePos = { -10, -10 };
		if (!screenToVr || !vr::GetPointerPose(vr::Side::Right).mIsValid)
		{
			return pointerMousePos;
		}

		const Vec2ui& targetSize = vr::GetRenderTargetSize();
		/*const */TFE_Settings_Vr* vrSettings = TFE_Settings::getVrSettings();

		const vr::Pose& ctrlLeft = vr::GetPointerPose(vr::Side::Left);
		const vr::Pose& ctrlRight = vr::GetPointerPose(vr::Side::Right);
		const vr::ControllerState& ctrlStateLeft = vr::GetControllerState(vr::Side::Left);
		const vr::ControllerState& ctrlStateRight = vr::GetControllerState(vr::Side::Right);

		const bool ctrlLeftValid = ctrlLeft.mIsValid;
		const bool ctrlRightValid = ctrlRight.mIsValid;
		Mat4 ctrlMtx[2] = { ctrlLeftValid ? ctrlLeft.mTransformation : TFE_Math::getIdentityMatrix4(), ctrlRightValid ? ctrlRight.mTransformation : TFE_Math::getIdentityMatrix4() };
		// no translation
		ctrlMtx[0].m3 = Vec4f{ 0.0f, 0.0f, 0.0f, 1.0f };
		ctrlMtx[1].m3 = Vec4f{ 0.0f, 0.0f, 0.0f, 1.0f };
		const float ctrlGripTrigger[2] = { ctrlLeftValid ? ctrlStateLeft.mHandTrigger : 0.0f, ctrlRightValid ? ctrlStateRight.mHandTrigger : 0.0f };
		const float ctrlIndexTrigger[2] = { ctrlLeftValid ? ctrlStateLeft.mIndexTrigger : 0.0f, ctrlRightValid ? ctrlStateRight.mIndexTrigger : 0.0f };

		using UIPlaneIntersection = std::pair<std::optional<TFE_Math::RayPlaneIntersection>, Vec2f>;
		auto GetUIPlaneIntersection = [&](const std::array<Vec3f, 8>& frustum, const Mat4& ltw, const Vec3f& shift, float planeDistance, const Vec3f& handPos, const Vec3f& handAt, const Vec2ui& displaySize) -> UIPlaneIntersection {
			enum Point
			{
				NearLeftBottom = 0,
				NearRightBottom = 1,
				NearLeftTop = 2,
				NearRightTop = 3,
				FarLeftBottom = 4,
				FarRightBottom = 5,
				FarLeftTop = 6,
				FarRightTop = 7,
				NearAt = 8,
				FarAt = 9
			};

			// to world space
			std::array<Vec3f, 10> points;
			for (size_t i = 0; i < frustum.size(); i++)
			{
				points[i] = frustum[i];
				if (i >= 4) // far plane points
				{
					points[i].z = -planeDistance;
				}

				Vec3f c = points[i] + shift;
				Mat4 m = ltw;
				Vec4f wc = TFE_Math::multiply(m, Vec4f{ c.x, c.y, c.z, 1.0f });
				points[i] = Vec3f{ wc.x, wc.y, wc.z };
			}
			points[NearAt] = 0.5f * (points[Point::NearLeftBottom] + points[Point::NearRightTop]);
			points[FarAt] = 0.5f * (points[Point::FarLeftBottom] + points[Point::FarRightTop]);

			const Vec3f& planePoint = points[FarAt];
			const Vec3f planeNormal = TFE_Math::normalize(points[FarAt] - points[NearAt]);

			// compute intersection between projected UI plane & hand pointer then compute screen pos
			Vec2f screenPos;
			const std::optional<TFE_Math::RayPlaneIntersection> intersection = TFE_Math::getRayPlaneIntersection(handPos, handAt, planePoint, planeNormal);
			if (intersection && intersection->t > 0.001f)
			{
				const Vec3f& p = intersection->point;
				// ax + by + cz + d = 0
				float dx = -(points[FarLeftTop] | (points[FarRightTop] - points[FarLeftTop]));
				float sidePlaneX = (p | (points[FarRightTop] - points[FarLeftTop])) + dx;
				float dy = -(points[FarLeftTop] | (points[FarLeftBottom] - points[FarLeftTop]));
				float sidePlaneY = (p | (points[FarLeftBottom] - points[FarLeftTop])) + dy;
				const Vec2f signs{ TFE_Math::sign(sidePlaneX), TFE_Math::sign(sidePlaneY) };

				const float dLeft = TFE_Math::getLinePointDistance(points[FarLeftBottom], points[FarLeftTop], p);
				const float dTop = TFE_Math::getLinePointDistance(points[FarLeftTop], points[FarRightTop], p);
				const Vec2f screenRelPos{ dLeft / TFE_Math::length(points[FarRightTop] - points[FarLeftTop]), dTop / TFE_Math::length(points[FarLeftTop] - points[FarLeftBottom]) };
				screenPos = Vec2f{ screenRelPos.x * displaySize.x, screenRelPos.y * displaySize.y } *signs;

				vrSettings->debug.handPos = handPos;
				vrSettings->debug.handAt = handAt;
				vrSettings->debug.intersection = intersection->point;
				vrSettings->debug.dx = dx;
				vrSettings->debug.dy = dy;
				vrSettings->debug.dLeft = dLeft;
				vrSettings->debug.dTop = dTop;
				vrSettings->debug.sidePlaneX = sidePlaneX;
				vrSettings->debug.sidePlaneY = sidePlaneY;
				vrSettings->debug.screenPos = screenPos;
			}

			return { intersection, screenPos };
		};

		// must fit shader imGui.vert computation:
		// float leftTrigger = CtrlIndexTrigger[0] > 0.0 ? CtrlIndexTrigger[0] : CtrlGripTrigger[0];
		// vec3 shift = vec3(Shift.x, Shift.y, (1.0 - leftTrigger) * Shift.z);
		// vec3 pos = ProjectTo3D(vec2(vtx_pos.x, ScreenSize.y - vtx_pos.y), ScreenSize, Shift.w, Frustum) + shift;
		const float leftTrigger = screenToVr->allowZoomToCamera ? ctrlIndexTrigger[vr::Side::Left] : 0.0f;
		const Vec3f shift = { screenToVr->shift.x, screenToVr->shift.y, (1.0f - leftTrigger) * screenToVr->shift.z };
		const float distance = screenToVr->distance;
		const Vec3f at = -TFE_Math::normalize(TFE_Math::getVec3(ctrlRight.mTransformation.m2));
		auto [intersection, screenPos] = GetUIPlaneIntersection(vr::GetUnitedFrustum(), screenToVr->lockToCamera ? TFE_Math::getIdentityMatrix4() : ctrlMtx[vr::Side::Left], shift, distance, { 0.0f, 0.0f, 0.0f }, at, targetSize);
		if (intersection)
		{
			//TFE_INFO("VR", "mouse [{}, {}], pointer [{}, {}]", io.MousePos.x, io.MousePos.y, screenPos.x, screenPos.y);
			screenPos.x = std::clamp(screenPos.x, 0.0f, f32(targetSize.x - 1));
			screenPos.y = std::clamp(screenPos.y, 0.0f, f32(targetSize.y - 1));
			pointerMousePos = { (s32)screenPos.x, (s32)screenPos.y };
		}

		return pointerMousePos;
	}

	bool HandleControllerEvents(IGame::State gameState, s32& mouseRelX, s32& mouseRelY)
	{
		if (!IsInitialized())
		{
			return false;
		}

		bool mouseMoveEmulated = mPointerPose[Side::Right].mIsValid;

		auto UpdateButton = [](bool pressed, Button tfeButton) {
			if (pressed)
			{
				if (!mControllerButtonPressed[tfeButton])
				{
					TFE_Input::setButtonDown(tfeButton);
					//TFE_INFO("VR", "Button down : {}", (int)tfeButton);
				}
				mControllerButtonPressed[tfeButton] = true;
			}
			else
			{
				if (mControllerButtonPressed[tfeButton])
				{
					TFE_Input::setButtonUp(tfeButton);
					//TFE_INFO("VR", "Button up : {}", (int)tfeButton);
				}
				mControllerButtonPressed[tfeButton] = false;
			}
		};

		enum class Section
		{
			Unknown,
			Up,
			Down,
			Left,
			Right
		};

		auto UpdateThumbSection = [](bool pressed, ThumbSection thumbSection) {
			ThumbSectionState& state = mThumbSectionState[(size_t)thumbSection];
			if (pressed)
			{
				if (state == ThumbSectionState::None || state == ThumbSectionState::Up)
				{
					state = ThumbSectionState::Down;
					//TFE_INFO("VR", "Thumb section down: {}", (int)thumbSection);
				}
				else if (state == ThumbSectionState::Down)
				{
					state = ThumbSectionState::Pressed;
					//TFE_INFO("VR", "Thumb section pressed: {}", (int)thumbSection);
				}
			}
			else
			{
				if (state == ThumbSectionState::Down || state == ThumbSectionState::Pressed)
				{
					state = ThumbSectionState::Up;
					//TFE_INFO("VR", "Thumb section up: {}", (int)thumbSection);
				}
				else if (state == ThumbSectionState::Up)
				{
					state = ThumbSectionState::None;
					//TFE_INFO("VR", "Thumb section none: {}", (int)thumbSection);
				}
			}
		};

		auto GetThumbSection = [](const Vec2f& thumb, float minLength = 0.5f) {
			const float cosa = TFE_Math::dot(TFE_Math::normalize(thumb), { 0.0f, 1.0f });
			const float angle = TFE_Math::degrees(std::acosf(cosa));
			const float len = TFE_Math::length(thumb);
			//TFE_INFO("VR", "thumb: {}, angle: {}", thumb, angle);
			if (len >= minLength)
			{
				if (thumb.y > 0.0f && angle < 45.0f)
				{
					//TFE_INFO("VR", "Section::Up - thumb: {}, len: {}, angle: {}", thumb, len, angle);
					return Section::Up;
				}
				else if (thumb.y < 0.0f && angle > 135.0f)
				{
					//TFE_INFO("VR", "Section::Down - thumb: {}, len: {}, angle: {}", thumb, len, angle);
					return Section::Down;
				}
				else if (thumb.x < 0.0f && (angle >= 45.0f && angle <= 135.0f))
				{
					//TFE_INFO("VR", "Section::Left - thumb: {}, len: {}, angle: {}", thumb, len, angle);
					return Section::Left;
				}
				else if (thumb.x > 0.0f && (angle >= 45.0f && angle <= 135.0f))
				{
					//TFE_INFO("VR", "Section::Right - thumb: {}, len: {}, angle: {}", thumb, len, angle);
					return Section::Right;
				}
			}
			return Section::Unknown;
		};

		TFE_Settings_Vr* vrSettings = TFE_Settings::getVrSettings();
		const vr::ControllerState& controllerLeft = vr::GetControllerState(vr::Side::Left);
		const vr::ControllerState& controllerRight = vr::GetControllerState(vr::Side::Right);
		const float deadzone = 0.1f;

		// buttons
		UpdateButton(controllerLeft.mButtons& ControllerButton::A, Button::CONTROLLER_BUTTON_X);
		UpdateButton(controllerLeft.mButtons& ControllerButton::B, Button::CONTROLLER_BUTTON_Y);
		UpdateButton(controllerLeft.mButtons& ControllerButton::Menu, Button::CONTROLLER_BUTTON_GUIDE);
		UpdateButton(controllerLeft.mButtons& ControllerButton::Thumb, Button::CONTROLLER_BUTTON_LEFTSTICK);

		UpdateButton(controllerRight.mButtons& ControllerButton::A, Button::CONTROLLER_BUTTON_A);
		UpdateButton(controllerRight.mButtons& ControllerButton::B, Button::CONTROLLER_BUTTON_B);
		UpdateButton(controllerRight.mButtons& ControllerButton::Thumb, Button::CONTROLLER_BUTTON_RIGHTSTICK);

		// thumb sections
		const Section thumbSectionLeft = GetThumbSection(controllerLeft.mThumbStick);
		const Section thumbSectionRight = GetThumbSection(controllerRight.mThumbStick);
		UpdateThumbSection(thumbSectionLeft == Section::Up, ThumbSection::LeftThumbUp);
		UpdateThumbSection(thumbSectionLeft == Section::Down, ThumbSection::LeftThumbDown);
		UpdateThumbSection(thumbSectionLeft == Section::Left, ThumbSection::LeftThumbLeft);
		UpdateThumbSection(thumbSectionLeft == Section::Right, ThumbSection::LeftThumbRight);
		if (thumbSectionLeft == Section::Unknown)
		{
			UpdateThumbSection(false, ThumbSection::LeftThumbUp);
			UpdateThumbSection(false, ThumbSection::LeftThumbDown);
			UpdateThumbSection(false, ThumbSection::LeftThumbLeft);
			UpdateThumbSection(false, ThumbSection::LeftThumbRight);
		}
		UpdateThumbSection(thumbSectionRight == Section::Up, ThumbSection::RightThumbUp);
		UpdateThumbSection(thumbSectionRight == Section::Down, ThumbSection::RightThumbDown);
		UpdateThumbSection(thumbSectionRight == Section::Left, ThumbSection::RightThumbLeft);
		UpdateThumbSection(thumbSectionRight == Section::Right, ThumbSection::RightThumbRight);
		if (thumbSectionRight == Section::Unknown)
		{
			UpdateThumbSection(false, ThumbSection::RightThumbUp);
			UpdateThumbSection(false, ThumbSection::RightThumbDown);
			UpdateThumbSection(false, ThumbSection::RightThumbLeft);
			UpdateThumbSection(false, ThumbSection::RightThumbRight);
		}

		// triggers
		float indexTriggerLeft = 0.0f;
		float indexTriggerRight = 0.0f;
		if ((controllerLeft.mIndexTrigger < -deadzone) || (controllerLeft.mIndexTrigger > deadzone))
		{
			indexTriggerLeft = controllerLeft.mIndexTrigger;
			TFE_Input::setAxis(AXIS_LEFT_TRIGGER, controllerLeft.mIndexTrigger);
		}
		else
		{
			TFE_Input::setAxis(AXIS_LEFT_TRIGGER, 0.0f);
		}

		if ((controllerRight.mIndexTrigger < -deadzone) || (controllerRight.mIndexTrigger > deadzone))
		{
			indexTriggerRight = controllerRight.mIndexTrigger;
			TFE_Input::setAxis(AXIS_RIGHT_TRIGGER, controllerRight.mIndexTrigger);
		}
		else
		{
			TFE_Input::setAxis(AXIS_RIGHT_TRIGGER, 0.0f);
		}

		float handTriggerLeft = 0.0f;
		float handTriggerRight = 0.0f;
		if ((controllerLeft.mHandTrigger < -deadzone) || (controllerLeft.mHandTrigger > deadzone))
		{
			handTriggerLeft = controllerLeft.mHandTrigger;
		}

		if ((controllerRight.mHandTrigger < -deadzone) || (controllerRight.mHandTrigger > deadzone))
		{
			handTriggerRight = controllerRight.mHandTrigger;
		}

		// axis
		if ((controllerLeft.mButtons & ControllerButton::Thumb) == 0)
		{
			TFE_Input::setAxis(AXIS_LEFT_X, controllerLeft.mThumbStick.x);
			TFE_Input::setAxis(AXIS_LEFT_Y, controllerLeft.mThumbStick.y);
		}
		else
		{
			TFE_Input::setAxis(AXIS_LEFT_X, 0.0f);
			TFE_Input::setAxis(AXIS_LEFT_Y, 0.0f);
		}

		if (handTriggerLeft == 0.0f && (controllerRight.mButtons & ControllerButton::Thumb) == 0)
		{
			TFE_Input::setAxis(AXIS_RIGHT_X, controllerRight.mThumbStick.x);
			TFE_Input::setAxis(AXIS_RIGHT_Y, controllerRight.mThumbStick.y);
		}
		else
		{
			TFE_Input::setAxis(AXIS_RIGHT_X, 0.0f);
			TFE_Input::setAxis(AXIS_RIGHT_Y, 0.0f);
		}

		bool escapeDown = false;
		// shoulders & F1
		if (TFE_Math::length(controllerRight.mThumbStick) > 0.5f && handTriggerLeft > 0.5f)
		{
			const Section section = GetThumbSection(controllerRight.mThumbStick);
			switch (section)
			{
			case Section::Up:
				TFE_Input::setKeyDown(KeyboardCode::KEY_F1, false);
				TFE_Input::setBufferedKey(KeyboardCode::KEY_F1);
				break;
			case Section::Down:
				TFE_Input::setKeyDown(KeyboardCode::KEY_ESCAPE, false);
				TFE_Input::setBufferedKey(KeyboardCode::KEY_ESCAPE);
				escapeDown = true;
				break;
			case Section::Left:
				UpdateButton(true, Button::CONTROLLER_BUTTON_LEFTSHOULDER);
				break;
			case Section::Right:
				UpdateButton(true, Button::CONTROLLER_BUTTON_RIGHTSHOULDER);
				break;
			case Section::Unknown:
			default:
				break;
			}
		}
		else
		{
			UpdateButton(false, Button::CONTROLLER_BUTTON_LEFTSHOULDER);
			UpdateButton(false, Button::CONTROLLER_BUTTON_RIGHTSHOULDER);
			TFE_Input::setKeyUp(KeyboardCode::KEY_F1);
			TFE_Input::setKeyUp(KeyboardCode::KEY_ESCAPE);
		}

		// dpad
		if (TFE_Math::length(controllerLeft.mThumbStick) > 0.5f && TFE_Input::buttonPressed(Button::CONTROLLER_BUTTON_LEFTSTICK))
		{
			const Section section = GetThumbSection(controllerLeft.mThumbStick);
			switch (section)
			{
			case Section::Up:
				UpdateButton(true, Button::CONTROLLER_BUTTON_DPAD_UP);
				break;
			case Section::Down:
				UpdateButton(true, Button::CONTROLLER_BUTTON_DPAD_DOWN);
				break;
			case Section::Left:
				UpdateButton(true, Button::CONTROLLER_BUTTON_DPAD_LEFT);
				break;
			case Section::Right:
				UpdateButton(true, Button::CONTROLLER_BUTTON_DPAD_RIGHT);
				break;
			case Section::Unknown:
			default:
				break;
			}
		}
		else
		{
			UpdateButton(false, Button::CONTROLLER_BUTTON_DPAD_UP);
			UpdateButton(false, Button::CONTROLLER_BUTTON_DPAD_DOWN);
			UpdateButton(false, Button::CONTROLLER_BUTTON_DPAD_LEFT);
			UpdateButton(false, Button::CONTROLLER_BUTTON_DPAD_RIGHT);
		}

		const bool inMenu = gameState != IGame::State::Mission || TFE_FrontEndUI::isConfigMenuOpen() || TFE_DarkForces::escapeMenu_isOpen() || TFE_DarkForces::pda_isOpen();

		if (mouseRelX == 0 && mouseRelY == 0) // ignore if mouse is currently moving
		{
			// emulate mouse move & left mouse click if in any menu
			if (inMenu)
			{
				TFE_Settings_Vr::ScreenToVr* screenToVr = nullptr;

				if (TFE_FrontEndUI::isConfigMenuOpen())
				{
					screenToVr = &vrSettings->configToVr;
				}
				else if (TFE_DarkForces::escapeMenu_isOpen())
				{
					screenToVr = &vrSettings->menuToVr;
				}
				else if (TFE_DarkForces::pda_isOpen())
				{
					screenToVr = &vrSettings->pdaToVr;
				}
				else if (gameState == IGame::State::Cutscene || gameState == IGame::State::AgentMenu || gameState == IGame::State::Briefing)
				{
					screenToVr = &vrSettings->pdaToVr;
				}
				else if (gameState == IGame::State::Unknown)
				{
					screenToVr = &vrSettings->configToVr;
				}
				else
				{
					TFE_WARN("VR", "Unknown game state");
				}

				mPointerMousePos = GetMousePosByPointer(screenToVr);

				if (mPointerMousePos.x >= 0 && mPointerMousePos.y >= 0)
				{
					// ImGui UI needs mouse move events to update UI
					if (mPointerMousePos != mLastPointerMousePos)
					{
						SDL_Event event;
						event.type = SDL_MOUSEMOTION;
						event.motion.which = 0;
						event.motion.x = mPointerMousePos.x;
						event.motion.y = mPointerMousePos.y;
						mEmulatedEvents.push_back(event);
						mLastPointerMousePos = mPointerMousePos;
					}

					// emulate mouse wheel by right thumb stick
					if (!escapeDown)
					{
						const ThumbSectionState rightThumbUpSectionState = mThumbSectionState[(size_t)ThumbSection::RightThumbUp];
						const ThumbSectionState rightThumbDownSectionState = mThumbSectionState[(size_t)ThumbSection::RightThumbDown];
						const ThumbSectionState rightThumbRightSectionState = mThumbSectionState[(size_t)ThumbSection::RightThumbRight];
						const ThumbSectionState rightThumbLeftSectionState = mThumbSectionState[(size_t)ThumbSection::RightThumbLeft];
						if (rightThumbUpSectionState == ThumbSectionState::Down || rightThumbDownSectionState == ThumbSectionState::Down ||
							rightThumbRightSectionState == ThumbSectionState::Down || rightThumbLeftSectionState == ThumbSectionState::Down)
						{
							const float len = 3.0f * TFE_Math::length(controllerRight.mThumbStick);

							SDL_Event event;
							event.motion.which = 0;
							event.type = SDL_MOUSEWHEEL;

							event.wheel.preciseX = 0.0f;
							if (rightThumbRightSectionState == ThumbSectionState::Down)
							{
								event.wheel.preciseX = len;
							}
							else if (rightThumbLeftSectionState == ThumbSectionState::Down)
							{
								event.wheel.preciseX = -len;
							}
							event.wheel.x = (Sint32)event.wheel.preciseX;

							event.wheel.preciseY = 0;
							if (rightThumbUpSectionState == ThumbSectionState::Down)
							{
								event.wheel.preciseY = len;
							}
							else if (rightThumbDownSectionState == ThumbSectionState::Down)
							{
								event.wheel.preciseY = -len;
							}
							event.wheel.y = (Sint32)event.wheel.preciseY;

							mEmulatedEvents.push_back(event);
						}

						// other UIs need mouse position
						TFE_Input::setMousePos(mPointerMousePos.x, mPointerMousePos.y);
					}
				}

				// emulate left mouse click
				if (TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_A) || TFE_Input::getAxis(AXIS_RIGHT_TRIGGER) >= 0.25f)
				{
					if (!mMouseButtonsPressed)
					{
						SDL_Event event;
						event.type = SDL_MOUSEBUTTONDOWN;
						event.button.which = 0;
						event.button.button = SDL_BUTTON_LEFT;
						mMouseButtonsPressed = 1;
						mEmulatedEvents.push_back(event);

						//mouseMoveEmulated = true;

						//TFE_INFO("VR", "emulated SDL_MOUSEBUTTONDOWN");
					}
				}
				else
				{
					if (mMouseButtonsPressed)
					{
						SDL_Event event;
						event.button.button = SDL_BUTTON_LEFT;
						event.button.which = 0;
						event.type = SDL_MOUSEBUTTONUP;
						mMouseButtonsPressed = 0;
						mEmulatedEvents.push_back(event);

						//TFE_INFO("VR", "emulated SDL_MOUSEBUTTONUP");
					}
				}
			}
			else
			{	// in game, to rotate player emulate mouse movement by controller rotation
				const float sensitivityVertical = (vrSettings->rightControllerRotationInvertVertical ? -1.0f : 1.0f) * (vrSettings->rightControllerRotationSensitivityVertical * (1.0f + handTriggerRight));
				const float sensitivityHorizontal = (vrSettings->rightControllerRotationInvertHorizontal ? -1.0f : 1.0f) * (vrSettings->rightControllerRotationSensitivityHorizontal * (1.0f + handTriggerRight));

				if (handTriggerRight > 0.0f && mLastPointerPose[Side::Right].mIsValid && mPointerPose[Side::Right].mIsValid)
				{
					const Mat4& pointerLast = mLastPointerPose[Side::Right].mTransformation;
					const Mat4& pointer = mPointerPose[Side::Right].mTransformation;
					const Mat3 deltaRot = TFE_Math::getMatrix3(TFE_Math::mulMatrix4(pointer, TFE_Math::transpose4(pointerLast)));

					const float roll = std::atan2(deltaRot.m1.x, deltaRot.m0.x);
					const float pitch = std::atan2(deltaRot.m2.y, deltaRot.m2.z);
					const float yaw = std::asin(-deltaRot.m2.x);
					//TFE_INFO("VR", "[{}, {}, {}]", TFE_Math::degrees(yaw), TFE_Math::degrees(pitch), TFE_Math::degrees(roll));
					TFE_Input::setRelativeMousePos((s32)(TFE_Math::degrees(yaw) * sensitivityHorizontal), (s32)(TFE_Math::degrees(pitch) * sensitivityVertical));

					//mouseMoveEmulated = true;
				}
			}
		}

		//if (handTriggerRight == 0.0f)
		{
			mLastPointerPose[Side::Left] = mPointerPose[Side::Left];
			mLastPointerPose[Side::Right] = mPointerPose[Side::Right];
		}

		return mouseMoveEmulated;
	}

	Vec2i GetPointerMousePos()
	{
		return mPointerMousePos;
	}

	std::vector<SDL_Event> GetGeneratedSDLEvents()
	{
		std::vector<SDL_Event> events = std::move(mEmulatedEvents);
		return events;
	}
}
