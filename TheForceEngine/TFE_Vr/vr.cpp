#include "vr.h"
#include <TFE_System/system.h>
#include <vector>
#include "VrWrapper.h"

namespace vr
{
	vrw::VrWrapper* g_VrWrapper = nullptr;

	Vec2ui			 									mTargetSize{ 0, 0 };

	std::array<Pose, Side::Count>						mHandPose;
	std::array<ControllerState, Side::Count>			mControllerState;

	std::array<Mat4, Side::Count>						mProjection{ TFE_Math::getIdentityMatrix4(), TFE_Math::getIdentityMatrix4() };
	std::array<Mat4, Side::Count>						mProjectionYDown{ TFE_Math::getIdentityMatrix4(), TFE_Math::getIdentityMatrix4() };
	std::array<Pose, Side::Count>						mEyePose;
	Mat4												mUnitedProjection{ TFE_Math::getIdentityMatrix4() };
	Mat4												mUnitedProjectionDown{ TFE_Math::getIdentityMatrix4() };
	std::array<Vec3f, 8>								mUnitedFrustum;
	Vec2f												mFov{ 0.0f, 0.0f };
	float												mEyesDistance{ 0.0f };

	std::array<std::vector<std::unique_ptr<RenderTarget>>, Side::Count>	mRenderTargets;
	uint32_t											mSwapchainIndex{ 0 };

	void UpdateView(Side eye)
	{
		g_VrWrapper->UpdateView((vrw::VrWrapper::Side)eye);
		mSwapchainIndex = g_VrWrapper->GetSwapchainIndex();
	}

	void Commit(Side eye)
	{
		g_VrWrapper->Commit((vrw::VrWrapper::Side)eye);
	}

	void SubmitFrame()
	{
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

	UpdateStatus UpdateFrame(float cameraNear, float cameraFar)
	{
		const UpdateStatus status = (UpdateStatus)g_VrWrapper->UpdateFrame(cameraNear, cameraFar);
		if (status != UpdateStatus::ShouldQuit)
		{
			mProjection[Side::Left] = VrMat4ToTFEMat4(g_VrWrapper->GetEyeProj(vrw::VrWrapper::Side::Left, true));
			mProjection[Side::Right] = VrMat4ToTFEMat4(g_VrWrapper->GetEyeProj(vrw::VrWrapper::Side::Right, true));
			mProjectionYDown[Side::Left] = VrMat4ToTFEMat4(g_VrWrapper->GetEyeProj(vrw::VrWrapper::Side::Left, false));
			mProjectionYDown[Side::Right] = VrMat4ToTFEMat4(g_VrWrapper->GetEyeProj(vrw::VrWrapper::Side::Right, false));

			const vrw::VrWrapper::Pose& eyePoseLeft = g_VrWrapper->GetEyePose(vrw::VrWrapper::Side::Left);
			const vrw::VrWrapper::Pose& eyePoseRight = g_VrWrapper->GetEyePose(vrw::VrWrapper::Side::Right);
			mEyePose[Side::Left] = { VrMat4ToTFEMat4(eyePoseLeft.mTransformation), VrMat4ToTFEMat4(eyePoseLeft.mTransformationYDown), eyePoseLeft.mIsValid };
			mEyePose[Side::Right] = { VrMat4ToTFEMat4(eyePoseRight.mTransformation), VrMat4ToTFEMat4(eyePoseRight.mTransformationYDown), eyePoseRight.mIsValid };

			mUnitedProjection = VrMat4ToTFEMat4(g_VrWrapper->GetUnitedProj(true));
			mUnitedProjectionDown = VrMat4ToTFEMat4(g_VrWrapper->GetUnitedProj(false));

			const vrw::Vec3f* frustumPoints = g_VrWrapper->GetUnitedFrustum();
			for (size_t i = 0; i < 8; i++)
			{
				mUnitedFrustum[i] = { frustumPoints[i].x, frustumPoints[i].y, frustumPoints[i].z };
			}

			mFov = { g_VrWrapper->GetFov().x, g_VrWrapper->GetFov().y };
			mEyesDistance = g_VrWrapper->GetEyesDistance();
		}

		return status;
	}
}
