#include "touchInput.h"
#include <TFE_System/math.h>
#include <TFE_System/system.h>
#include <TFE_Ui/imGUI/imgui.h>
#include <vector>
#include <chrono>

extern bool s_inMenu;

namespace TFE_Input
{
	using Clock = std::chrono::high_resolution_clock;
	using TimePoint = Clock::time_point;
	using Seconds = std::chrono::duration<double>;

	class TouchContext
	{
	public:
		TouchContext(bool enable);

	public:
		bool IsEnabled() const;
		void Enable(bool enable);
		TimePoint GetLastTouchTime() const;
		Vec2f GetWindowSize() const;
		Vec2f GetAbsolutePos(const Vec2f& rel) const;
		float PixelsToCm(float pixels) const;
		Vec2f PixelsToCm(const Vec2f& pixels) const;
		float CmToPixels(float cms) const;
		Vec2f CmToPixels(const Vec2f& cms) const;

		void HandleEvents(const SDL_Event& event);

	private:
		bool mEnabled{ false };
		TimePoint mLastTouchTime;
		float mDpi = 400.0f;  // ~ fullHD on 6"

	public:
		std::unordered_map<FingerID, Finger> mActiveFingers;
	};

	std::vector<std::unique_ptr<TouchControl>> s_TouchControls;
	std::unique_ptr<TouchContext> s_TouchContext;

	void TouchAction::Perform(const TouchAction& action)
	{
		switch (action.mAction)
		{
		// mouse
		case Action::LeftMouseButtonDown:
			if (!TFE_Input::mouseDown(MouseButton::MBUTTON_LEFT))
			{
				TFE_Input::setMouseButtonDown(MouseButton::MBUTTON_LEFT);
			}
			break;
		case Action::LeftMouseButtonUp:
			if (TFE_Input::mouseDown(MouseButton::MBUTTON_LEFT))
			{
				TFE_Input::setMouseButtonUp(MouseButton::MBUTTON_LEFT);
			}
			break;
		case Action::RightMouseButtonDown:
			if (!TFE_Input::mouseDown(MouseButton::MBUTTON_RIGHT))
			{
				TFE_Input::setMouseButtonDown(MouseButton::MBUTTON_RIGHT);
			}
			break;
		case Action::RightMouseButtonUp:
			if (TFE_Input::mouseDown(MouseButton::MBUTTON_RIGHT))
			{
				TFE_Input::setMouseButtonUp(MouseButton::MBUTTON_RIGHT);
			}
			break;
		case Action::MiddleMouseButtonDown:
			if (!TFE_Input::mouseDown(MouseButton::MBUTTON_MIDDLE))
			{
				TFE_Input::setMouseButtonDown(MouseButton::MBUTTON_MIDDLE);
			}
			break;
		case Action::MiddleMouseButtonUp:
			if (TFE_Input::mouseDown(MouseButton::MBUTTON_MIDDLE))
			{
				TFE_Input::setMouseButtonUp(MouseButton::MBUTTON_MIDDLE);
			}
			break;
		case Action::SetRelativeMousePos:
		{
			const Vec2i& v = std::get<Vec2i>(action.mData);
			TFE_Input::setRelativeMousePos(v.x, v.y);
		} break;
		// controller
		case Action::LeftAxis:
		{
			const Vec2f &v = std::get<Vec2f>(action.mData);
			TFE_Input::setAxis(Axis::AXIS_LEFT_X, v.x);
			TFE_Input::setAxis(Axis::AXIS_LEFT_Y, v.y);
		} break;
		case Action::RightAxis:
		{
			const Vec2f& v = std::get<Vec2f>(action.mData);
			TFE_Input::setAxis(Axis::AXIS_RIGHT_X, v.x);
			TFE_Input::setAxis(Axis::AXIS_RIGHT_Y, v.y);
		} break;
		case Action::LeftTrigger:
		{
			const float& v = std::get<float>(action.mData);
			TFE_Input::setAxis(AXIS_LEFT_TRIGGER, v);
		} break;
		case Action::RightTrigger:
		{
			const float& v = std::get<float>(action.mData);
			TFE_Input::setAxis(AXIS_RIGHT_TRIGGER, v);
		} break;

		case Action::LeftShoulderDown:
			if (!TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_LEFTSHOULDER))
			{
				TFE_Input::setButtonDown(Button::CONTROLLER_BUTTON_LEFTSHOULDER);
			}
			break;
		case Action::LeftShoulderUp:
			if (TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_LEFTSHOULDER))
			{
				TFE_Input::setButtonUp(Button::CONTROLLER_BUTTON_LEFTSHOULDER);
			}
			break;
		case Action::RightShoulderDown:
			if (!TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_RIGHTSHOULDER))
			{
				TFE_Input::setButtonDown(Button::CONTROLLER_BUTTON_RIGHTSHOULDER);
			}
			break;
		case Action::RightShoulderUp:
			if (TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_RIGHTSHOULDER))
			{
				TFE_Input::setButtonUp(Button::CONTROLLER_BUTTON_RIGHTSHOULDER);
			}
			break;

		case Action::AButtonDown:
			if (!TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_A))
			{
				TFE_Input::setButtonDown(Button::CONTROLLER_BUTTON_A);
			}
			break;
		case Action::AButtonUp:
			if (TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_A))
			{
				TFE_Input::setButtonUp(Button::CONTROLLER_BUTTON_A);
			}
			break;
		case Action::BButtonDown:
			if (!TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_B))
			{
				TFE_Input::setButtonDown(Button::CONTROLLER_BUTTON_B);
			}
			break;
		case Action::BButtonUp:
			if (TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_B))
			{
				TFE_Input::setButtonUp(Button::CONTROLLER_BUTTON_B);
			}
			break;
		case Action::XButtonDown:
			if (!TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_X))
			{
				TFE_Input::setButtonDown(Button::CONTROLLER_BUTTON_X);
			}
			break;
		case Action::XButtonUp:
			if (TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_X))
			{
				TFE_Input::setButtonUp(Button::CONTROLLER_BUTTON_X);
			}
			break;
		case Action::YButtonDown:
			if (!TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_Y))
			{
				TFE_Input::setButtonDown(Button::CONTROLLER_BUTTON_Y);
			}
			break;
		case Action::YButtonUp:
			if (TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_Y))
			{
				TFE_Input::setButtonUp(Button::CONTROLLER_BUTTON_Y);
			}
			break;

		case Action::DPadLeftDown:
			if (!TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_DPAD_LEFT))
			{
				TFE_Input::setButtonDown(Button::CONTROLLER_BUTTON_DPAD_LEFT);
			}
			break;
		case Action::DPadLeftUp:
			if (TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_DPAD_LEFT))
			{
				TFE_Input::setButtonUp(Button::CONTROLLER_BUTTON_DPAD_LEFT);
			}
			break;
		case Action::DPadRightDown:
			if (!TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_DPAD_RIGHT))
			{
				TFE_Input::setButtonDown(Button::CONTROLLER_BUTTON_DPAD_RIGHT);
			}
			break;
		case Action::DPadRightUp:
			if (TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_DPAD_RIGHT))
			{
				TFE_Input::setButtonUp(Button::CONTROLLER_BUTTON_DPAD_RIGHT);
			}
			break;
		case Action::DPadTopDown:
			if (!TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_DPAD_UP))
			{
				TFE_Input::setButtonDown(Button::CONTROLLER_BUTTON_DPAD_UP);
			}
			break;
		case Action::DPadTopUp:
			if (TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_DPAD_UP))
			{
				TFE_Input::setButtonUp(Button::CONTROLLER_BUTTON_DPAD_UP);
			}
			break;
		case Action::DPadBottomDown:
			if (!TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_DPAD_DOWN))
			{
				TFE_Input::setButtonDown(Button::CONTROLLER_BUTTON_DPAD_DOWN);
			}
			break;
		case Action::DPadBottomUp:
			if (TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_DPAD_DOWN))
			{
				TFE_Input::setButtonUp(Button::CONTROLLER_BUTTON_DPAD_DOWN);
			}
			break;

		case Action::GuideDown:
			if (!TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_GUIDE))
			{
				TFE_Input::setButtonDown(Button::CONTROLLER_BUTTON_GUIDE);
			}
			break;
		case Action::GuideUp:
			if (TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_GUIDE))
			{
				TFE_Input::setButtonUp(Button::CONTROLLER_BUTTON_GUIDE);
			}
			break;
		case Action::StartDown:
			if (!TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_START))
			{
				TFE_Input::setButtonDown(Button::CONTROLLER_BUTTON_START);
			}
			break;
		case Action::StartUp:
			if (TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_START))
			{
				TFE_Input::setButtonUp(Button::CONTROLLER_BUTTON_START);
			}
			break;
		default:
			TFE_ERROR("TOUCH", "action {} not handled", EnumValue(action.mAction));
			break;
		}
	}

	void SetDefault()
	{
		s_TouchControls.clear();

		const Vec2f displaySize = s_TouchContext->GetWindowSize();
		const float aspectRatio = displaySize.x / displaySize.y;

		{	// move button
			constexpr float fromBottom = 0.2f; // cm
			constexpr float btnSize = 3.8f; // cm
			constexpr float btnSizeInner = 1.8f; // cm
			const float btnPosY = (displaySize.y - s_TouchContext->CmToPixels(fromBottom + btnSize / 2.0f)) / displaySize.y;
			const Vec2f btnPos = { 0.8f, btnPosY };
			auto moveControl = std::make_unique<MoveButton>("btn_move", Vec4f{ 0.0f, 0.0f, 0.5f, 1.0f }, btnSize, btnSizeInner,
				TouchControl::Actions {
					{ TouchControl::State::Clicked, TouchAction{ TouchAction::Action::LeftAxis, Vec2f{} } },
					{ TouchControl::State::Dragged, TouchAction{ TouchAction::Action::LeftAxis, Vec2f{} } },
					{ TouchControl::State::Released, TouchAction{ TouchAction::Action::LeftAxis, Vec2f{ 0.0f, 0.0f } } }
				});
			s_TouchControls.push_back(std::move(moveControl));
		}

		{	// rotate button
			auto rotateControl = std::make_unique<RotateButton>("btn_rotate", Vec4f{ 0.5f, 0.0f, 1.0f, 1.0f },
				TouchControl::Actions {
					{ TouchControl::State::Dragged, TouchAction{ TouchAction::Action::SetRelativeMousePos, Vec2i{} } },
					{ TouchControl::State::Released, TouchAction{ TouchAction::Action::SetRelativeMousePos, Vec2i{ 0, 0 } } }
				});
			s_TouchControls.push_back(std::move(rotateControl));
		}

		{	// fire button
			constexpr float fromBottom = 0.2f; // cm
			constexpr float btnSize = 2.0f; // cm
			const float btnsPosY = (displaySize.y - s_TouchContext->CmToPixels(fromBottom + btnSize / 2.0f)) / displaySize.y;
			const Vec2f btnPos = { 0.8f, btnsPosY };
			auto fireControl = std::make_unique<CircleButton>("btn_fire", Vec4f{ 0.0f, 0.0f, 1.0f, 1.0f }, btnPos, btnSize,
				TouchControl::Actions {
					{ TouchControl::State::Clicked, TouchAction{ TouchAction::Action::LeftMouseButtonDown, std::monostate{} } },
					{ TouchControl::State::Dragged, TouchAction{ TouchAction::Action::LeftMouseButtonDown, std::monostate{} } },
					{ TouchControl::State::Released, TouchAction{ TouchAction::Action::LeftMouseButtonUp, std::monostate{} } }
				});
			s_TouchControls.push_back(std::move(fireControl));
		}

		{	// shoulders
			constexpr float fromSide = 3.0f; // cm
			constexpr float fromTop = 0.1f; // cm
			constexpr float btnSize = 0.75f; // cm
			const float btnLeftPosX = (displaySize.x - s_TouchContext->CmToPixels(fromSide + btnSize / 2.0f)) / displaySize.x;
			const float btnRightPosX = s_TouchContext->CmToPixels(fromSide + btnSize / 2.0f) / displaySize.x;
			const float btnPosY = s_TouchContext->CmToPixels(fromTop + btnSize / 2.0f) / displaySize.y;
			const Vec2f btnLeftPos = { btnLeftPosX, btnPosY };
			const Vec2f btnRightPos = { btnRightPosX, btnPosY };
			auto shoulderLeftControl = std::make_unique<CircleButton>("btn_shoulder_left", Vec4f{ 0.0f, 0.0f, 1.0f, 1.0f }, btnLeftPos, btnSize,
				TouchControl::Actions {
					{ TouchControl::State::Clicked, TouchAction{ TouchAction::Action::LeftShoulderDown, std::monostate{} } },
					{ TouchControl::State::Released, TouchAction{ TouchAction::Action::LeftShoulderUp, std::monostate{} } }
				});
			s_TouchControls.push_back(std::move(shoulderLeftControl));
			auto shoulderRightControl = std::make_unique<CircleButton>("btn_shoulder_right", Vec4f{ 0.0f, 0.0f, 1.0f, 1.0f }, btnRightPos, btnSize,
				TouchControl::Actions {
					{ TouchControl::State::Clicked, TouchAction{ TouchAction::Action::RightShoulderDown, std::monostate{} } },
					{ TouchControl::State::Released, TouchAction{ TouchAction::Action::RightShoulderUp, std::monostate{} } }
				});
			s_TouchControls.push_back(std::move(shoulderRightControl));
		}

		{	// guide
			constexpr float fromRight = 0.1f; // cm & from top
			constexpr float btnSize = 0.75f; // cm
			const float btnPosX = s_TouchContext->CmToPixels(fromRight + btnSize / 2.0f) / displaySize.x;
			const Vec2f btnPos = { btnPosX, btnPosX * aspectRatio };
			auto guideControl = std::make_unique<CircleButton>("btn_guide", Vec4f{ 0.0f, 0.0f, 1.0f, 1.0f }, btnPos, btnSize,
				TouchControl::Actions {
					{ TouchControl::State::Clicked, TouchAction{ TouchAction::Action::GuideDown, std::monostate{} } },
					{ TouchControl::State::Released, TouchAction{ TouchAction::Action::GuideUp, std::monostate{} } }
				});
			s_TouchControls.push_back(std::move(guideControl));
		}

		{	// start
			constexpr float fromLeft = 0.1f; // cm & from top
			constexpr float btnSize = 0.75f; // cm
			const float btnPosX = (displaySize.x - s_TouchContext->CmToPixels(fromLeft + btnSize / 2.0f)) / displaySize.x;
			const float btnPosY = s_TouchContext->CmToPixels(fromLeft + btnSize / 2.0f) / displaySize.y;
			const Vec2f btnPos = { btnPosX, btnPosY };
			auto startControl = std::make_unique<CircleButton>("btn_start", Vec4f{ 0.0f, 0.0f, 1.0f, 1.0f }, btnPos, btnSize,
				TouchControl::Actions {
					{ TouchControl::State::Clicked, TouchAction{ TouchAction::Action::StartDown, std::monostate{} } },
					{ TouchControl::State::Released, TouchAction{ TouchAction::Action::StartUp, std::monostate{} } }
				});
			s_TouchControls.push_back(std::move(startControl));
		}

		{	// buttons A,B,X,Y
			constexpr float fromLeft = 0.2f; // cm
			constexpr float buttonsDistance = 0.15f; // cm
			constexpr float btnSize = 1.0f; // cm
			const float distance = (buttonsDistance + btnSize) / std::sqrtf(2.0f);
			const float btnShiftX = (s_TouchContext->CmToPixels(distance)) / displaySize.x;
			const float btnsPosX = (displaySize.x - s_TouchContext->CmToPixels(fromLeft + btnSize / 2.0f + distance)) / displaySize.x;
			const Vec2f btnPos = { btnsPosX, 0.3f };

			auto jumpControl = std::make_unique<CircleButton>("btn_jump", Vec4f{ 0.0f, 0.0f, 1.0f, 1.0f }, btnPos + Vec2f{ 0.0f, btnShiftX * aspectRatio }, btnSize,
				TouchControl::Actions {
					{ TouchControl::State::Clicked, TouchAction{ TouchAction::Action::AButtonDown, std::monostate{} } },
					{ TouchControl::State::Released, TouchAction{ TouchAction::Action::AButtonUp, std::monostate{} } }
				});
			s_TouchControls.push_back(std::move(jumpControl));

			auto runControl = std::make_unique<CircleButton>("btn_run", Vec4f{ 0.0f, 0.0f, 1.0f, 1.0f }, btnPos + Vec2f{ btnShiftX, 0.0f }, btnSize,
				TouchControl::Actions {
					{ TouchControl::State::Clicked, TouchAction{ TouchAction::Action::BButtonDown, std::monostate{} } },
					{ TouchControl::State::Released, TouchAction{ TouchAction::Action::BButtonUp, std::monostate{} } }
				});
			s_TouchControls.push_back(std::move(runControl));

			auto duckControl = std::make_unique<CircleButton>("btn_duck", Vec4f{ 0.0f, 0.0f, 1.0f, 1.0f }, btnPos + Vec2f{ -btnShiftX, 0.0f }, btnSize,
				TouchControl::Actions {
					{ TouchControl::State::Clicked, TouchAction{ TouchAction::Action::XButtonDown, std::monostate{} } },
					{ TouchControl::State::Released, TouchAction{ TouchAction::Action::XButtonUp, std::monostate{} } }
				});
			s_TouchControls.push_back(std::move(duckControl));

			auto openControl = std::make_unique<CircleButton>("btn_open", Vec4f{ 0.0f, 0.0f, 1.0f, 1.0f }, btnPos + Vec2f{ 0.0f, -btnShiftX * aspectRatio}, btnSize,
				TouchControl::Actions {
					{ TouchControl::State::Clicked, TouchAction{ TouchAction::Action::YButtonDown, std::monostate{} } },
					{ TouchControl::State::Released, TouchAction{ TouchAction::Action::YButtonUp, std::monostate{} } }
				});
			s_TouchControls.push_back(std::move(openControl));
		}

		{	// dpad
			constexpr float fromRight = 0.5f; // cm
			constexpr float buttonsDistance = 0.15f; // cm
			constexpr float btnSize = 0.75f; // cm
			const float distance = (buttonsDistance + btnSize) / std::sqrtf(2.0f);
			const float btnShiftX = (s_TouchContext->CmToPixels(distance)) / displaySize.x;
			const float btnsPosX = s_TouchContext->CmToPixels(fromRight + btnSize / 2.0f + distance) / displaySize.x;
			const Vec2f btnPos = { btnsPosX, 0.3f };

			auto dpadBottomControl = std::make_unique<CircleButton>("btn_dpad_bottom", Vec4f{ 0.0f, 0.0f, 1.0f, 1.0f }, btnPos + Vec2f{ 0.0f, btnShiftX * aspectRatio }, btnSize,
				TouchControl::Actions {
					{ TouchControl::State::Clicked, TouchAction{ TouchAction::Action::DPadBottomDown, std::monostate{} } },
					{ TouchControl::State::Released, TouchAction{ TouchAction::Action::DPadBottomUp, std::monostate{} } }
				});
			s_TouchControls.push_back(std::move(dpadBottomControl));

			auto dpadRightControl = std::make_unique<CircleButton>("btn_dpad_right", Vec4f{ 0.0f, 0.0f, 1.0f, 1.0f }, btnPos + Vec2f{ btnShiftX, 0.0f }, btnSize,
				TouchControl::Actions {
					{ TouchControl::State::Clicked, TouchAction{ TouchAction::Action::DPadRightDown, std::monostate{} } },
					{ TouchControl::State::Released, TouchAction{ TouchAction::Action::DPadRightUp, std::monostate{} } }
				});
			s_TouchControls.push_back(std::move(dpadRightControl));

			auto dpadLeftControl = std::make_unique<CircleButton>("btn_dpad_left", Vec4f{ 0.0f, 0.0f, 1.0f, 1.0f }, btnPos + Vec2f{ -btnShiftX, 0.0f }, btnSize,
				TouchControl::Actions {
					{ TouchControl::State::Clicked, TouchAction{ TouchAction::Action::DPadLeftDown, std::monostate{} } },
					{ TouchControl::State::Released, TouchAction{ TouchAction::Action::DPadLeftUp, std::monostate{} } }
				});
			s_TouchControls.push_back(std::move(dpadLeftControl));

			auto dpadTopControl = std::make_unique<CircleButton>("btn_dpad_top", Vec4f{ 0.0f, 0.0f, 1.0f, 1.0f }, btnPos + Vec2f{ 0.0f, -btnShiftX * aspectRatio}, btnSize,
				TouchControl::Actions {
					{ TouchControl::State::Clicked, TouchAction{ TouchAction::Action::DPadTopDown, std::monostate{} } },
					{ TouchControl::State::Released, TouchAction{ TouchAction::Action::DPadTopUp, std::monostate{} } }
				});
			s_TouchControls.push_back(std::move(dpadTopControl));
		}
	}

	s32 getNumTouchDevices()
	{
		return SDL_GetNumTouchDevices();
	}

	void initTouchControls(bool enable)
	{
		s_TouchContext = std::make_unique<TouchContext>(enable);
		SetDefault();
	}

	bool isTouchControlsEnabled()
	{
		return s_TouchContext && s_TouchContext->IsEnabled();
	}

	void enableTouchControls(bool enable)
	{
		if (s_TouchContext)
		{
			s_TouchContext->Enable(enable);
		}
	}

	void setDefaultTouchControls()
	{
		SetDefault();
	}

	void destroyTouchControls()
	{
		s_TouchControls.clear();
		s_TouchContext.reset();
	}

	void handleTouchEvents(const SDL_Event& event)
	{
		if (!s_inMenu && s_TouchContext && s_TouchContext->IsEnabled())
		{
			s_TouchContext->HandleEvents(event);
		}
	}

	void drawTouchControls()
	{
		if (s_inMenu || !s_TouchContext || !s_TouchContext->IsEnabled())
		{
			return;
		}

		TimePoint now = Clock::now();
		Seconds nonTouchTime = now - s_TouchContext->GetLastTouchTime();
		if (nonTouchTime.count() > 5.0)
		{
			return;
		}

		//ImGuiIO& io = ImGui::GetIO();
		const Vec2f displaySize = s_TouchContext->GetWindowSize();
		//TFE_INFO("TOUCH", "displaySize=[{}, {}]", displaySize.x, displaySize.y);

		const uint32_t windowFlags = ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground;
		//ImGui::PushFont(fpsFont);
		ImGui::SetNextWindowSize(displaySize);
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::Begin("##TOUCH_WINDOW", nullptr, windowFlags);

		for (auto& control : s_TouchControls)
		{
			control->Draw();
		}

		ImGui::End();
		//ImGui::PopFont();
	}

	TouchContext::TouchContext(bool enable)
		: mEnabled{ enable }
		, mLastTouchTime{ Clock::now() }
	{
		SDL_GetDisplayDPI(0, nullptr, &mDpi, nullptr);

		TFE_INFO("TOUCH", "DPI={}", mDpi);
	}

	bool TouchContext::IsEnabled() const
	{
		return mEnabled;
	}

	void TouchContext::Enable(bool enable)
	{
		mEnabled = enable;
	}

	TimePoint TouchContext::GetLastTouchTime() const
	{
		return mLastTouchTime;
	}

	Vec2f TouchContext::GetWindowSize() const
	{
		int w, h;
		SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &w, &h);
		return { (float)w, (float)h };
	}

	Vec2f TouchContext::GetAbsolutePos(const Vec2f& rel) const
	{
		return GetWindowSize() * rel;
	}

	float TouchContext::PixelsToCm(float pixels) const
	{
		return (2.54f / mDpi) * pixels;
	}

	Vec2f TouchContext::PixelsToCm(const Vec2f& pixels) const
	{
		return { PixelsToCm(pixels.x), PixelsToCm(pixels.y) };
	}

	float TouchContext::CmToPixels(float cms) const
	{
		return (mDpi / 2.54f) * cms;
	}

	Vec2f TouchContext::CmToPixels(const Vec2f& cms) const
	{
		return { CmToPixels(cms.x), CmToPixels(cms.y) };
	}

	void TouchContext::HandleEvents(const SDL_Event& event)
	{
		if (!mEnabled)
		{
			return;
		}

		mLastTouchTime = Clock::now();

		const SDL_TouchFingerEvent& finger = event.tfinger;

		//TFE_INFO("TOUCH", "finger {} event", finger.fingerId);

		switch (event.type)
		{
			case SDL_FINGERDOWN:
			{
				if (auto it = mActiveFingers.find(finger.fingerId); it != mActiveFingers.end())
				{
					TFE_ERROR("TOUCH", "finger {} is already down!", finger.fingerId);
					return;
				}

				Vec2f pos{ finger.x, finger.y };
				Finger newFinger{ finger.fingerId, Finger::State::Down, pos, pos, pos };
				auto p = mActiveFingers.emplace(finger.fingerId, newFinger );
				for (auto& control : s_TouchControls)
				{
					const Finger& f = p.first->second;
					control->HandleEvent(f);
				}
			} break;
			case SDL_FINGERUP:
			case SDL_FINGERMOTION:
			{
				if (auto it = mActiveFingers.find(finger.fingerId); it != mActiveFingers.end())
				{
					it->second.mLastPos = it->second.mCurrentPos;
					it->second.mCurrentPos = { finger.x, finger.y };
					it->second.mState = event.type == SDL_FINGERUP ? Finger::State::Up : Finger::State::Motion;
					for (auto& control : s_TouchControls)
					{
						const Finger& f = it->second;
						control->HandleEvent(f);
					}
					if (event.type == SDL_FINGERUP)
					{
						mActiveFingers.erase(finger.fingerId);
					}
				}
				else
				{
					TFE_ERROR("TOUCH", "finger {} not registered!", finger.fingerId);
				}
			} break;
			default:
				break;
		}
	}

	TouchControl::TouchControl(std::string name, const Vec4f& activeArea, const Actions& actions)
		: mName{ std::move(name) }
		, mActiveArea{ activeArea }
		, mActions{ actions }
	{}

	bool TouchControl::HandleEvent(const Finger& finger)
	{
		const bool printInfo = false;
		switch (finger.mState)
		{
			case Finger::State::Down:
			{
				if (mFingerID == InvalidFingerID)
				{
					if (IsHit(finger))
					{
						mState = State::Clicked;
						mFingerID = finger.mID;
						if (printInfo) TFE_INFO("TOUCH", "{} is clicked by finger {}", mName, mFingerID);
					}
				}
			} break;
			case Finger::State::Motion:
			{
				if (mFingerID == finger.mID)
				{
					mState = State::Dragged;
					if (printInfo) TFE_INFO("TOUCH", "{} is dragged by finger {}", mName, mFingerID);
				}
				else if (mFingerID == InvalidFingerID)
				{
					if (IsHit(finger))
					{
						mState = State::Hovered;
						if (printInfo) TFE_INFO("TOUCH", "{} is hovered by finger {}", mName, finger.mID);
					}
					else if (mState == State::Hovered)
					{
						mState = State::None;
						if (printInfo) TFE_INFO("TOUCH", "{} is unhovered by finger {}", mName, finger.mID);
					}
				}
			} break;
			case Finger::State::Up:
			{
				if (mFingerID == finger.mID)
				{
					mState = State::Released;
					//mFingerID = InvalidFingerID;
					if (printInfo) TFE_INFO("TOUCH", "{} is released by finger {}", mName, finger.mID);
				}
				else if (mState == State::Hovered)
				{
					mState = State::Unhovered;
					if (printInfo) TFE_INFO("TOUCH", "{} is unhovered by finger {}", mName, finger.mID);
				}
			} break;
			default:
				TFE_ERROR("TOUCH", "{}, finger state {} not handled", mName, EnumValue(finger.mState));
				break;
		}

		if (auto it = mActions.find(mState); it != mActions.end())
		{
			UpdateAction(finger, mState, it->second);
			TouchAction::Perform(it->second);
			if (finger.mState == Finger::State::Up)
			{
				if (mFingerID == finger.mID)
				{
					mFingerID = InvalidFingerID;
				}
			}
		}

		if (mState == State::Released || mState == State::Unhovered)
		{
			mState = State::None;
		}

		return true;
	}

	CircleButton::CircleButton(std::string name, const Vec4f& activeArea, const Vec2f& pos, float size, const Actions& actions)
		: TouchControl{ std::move(name), activeArea, actions }
		, mPos{ pos }
		, mSize{ size }
	{
	}

	bool CircleButton::IsHit(const Finger& finger)
	{
		const Vec2f touchPos = s_TouchContext->GetAbsolutePos(finger.mCurrentPos);
		const Vec2f buttonPos = s_TouchContext->GetAbsolutePos(mPos);
		const Vec2f diffInPx = touchPos - buttonPos;
		const Vec2f diffInCm = s_TouchContext->PixelsToCm(diffInPx);
		const float len = TFE_Math::length(diffInCm);
		if (len <= mSize / 2.0f)
		{
			//TFE_INFO("TOUCH", "{} is hit by finger {}", mName, finger.mID);
			return true;
		}
		return false;
	}

	void DrawCircleButton(const char* label, const Vec2f& center, float radius, ImU32 color)
	{
		ImGui::GetWindowDrawList()->AddCircleFilled(center, radius, color, 32);

		ImGui::SetCursorScreenPos(center - radius);
		ImGui::InvisibleButton(label, ImVec2(2 * radius, 2 * radius));

//		bool hovered = ImGui::IsItemHovered();
//		bool clicked = ImGui::IsItemClicked();
//
//		if (hovered)
//		{
//			ImGui::GetWindowDrawList()->AddCircleFilled(center, radius, hoverColor, 32);
//		}
//
//		return clicked;
	}

	void CircleButton::Draw()
	{
		const Vec2f p = s_TouchContext->GetAbsolutePos(mPos);
		DrawCircleButton(mName.c_str(), p, s_TouchContext->CmToPixels(mSize / 2.0f), IM_COL32(120, 120, 160, 30));
	}

	MoveButton::MoveButton(std::string name, const Vec4f& activeArea, float size, float innerSize, const Actions& actions)
		: CircleButton{ std::move(name), activeArea, {}, size, actions }
		, mInnerSize{ innerSize }
	{
	}

	bool MoveButton::IsHit(const Finger& finger)
	{
		if (finger.mState == Finger::State::Down &&
			mFingerID == InvalidFingerID &&
			finger.mStartPos.x >= mActiveArea.x && finger.mStartPos.x <= mActiveArea.z &&
			finger.mStartPos.y >= mActiveArea.y && finger.mStartPos.y <= mActiveArea.w)
		{
			mPos = finger.mStartPos;
			//TFE_INFO("TOUCH", "{} is hit by finger {}", mName, finger.mID);
			return true;
		}

		return false;
	}

	void MoveButton::UpdateAction(const Finger& finger, TouchControl::State state, TouchAction& action)
	{
		if (mFingerID == finger.mID)
		{
			if (state == State::Dragged)
			{
				const Vec2f startPos = s_TouchContext->GetAbsolutePos(finger.mStartPos);
				const Vec2f currentPos = s_TouchContext->GetAbsolutePos(finger.mCurrentPos);
				const Vec2f diffInPx = currentPos - startPos;
				const Vec2f diffInCm = s_TouchContext->PixelsToCm(diffInPx);
				const float len = TFE_Math::length(diffInCm);

				{
					if (len > mSize / 2.0f)
					{	// running
						if (!TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_B))
						{
							TFE_Input::setButtonDown(Button::CONTROLLER_BUTTON_B);
						}
					}
					else
					{
						if (TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_B))
						{
							TFE_Input::setButtonUp(Button::CONTROLLER_BUTTON_B);
						}
					}
				}

				Vec2f diffrel = diffInCm / mInnerSize;
				if (TFE_Math::length(diffrel) > 1.0f)
				{
					diffrel = TFE_Math::normalize(diffrel);
				}

				action.mData = Vec2f{diffrel.x, -diffrel.y};
			}
			else if (state == State::Released)
			{
				if (TFE_Input::buttonDown(Button::CONTROLLER_BUTTON_B))
				{
					TFE_Input::setButtonUp(Button::CONTROLLER_BUTTON_B);
				}
			}
		}
	}

	void MoveButton::Draw()
	{
		if (mFingerID == InvalidFingerID)
		{
			return;
		}

		const Vec2f p = s_TouchContext->GetAbsolutePos(mPos);
		DrawCircleButton(mName.c_str(), p, s_TouchContext->CmToPixels(mSize / 2.0f), IM_COL32(120, 120, 160, 25));
		DrawCircleButton(mName.c_str(), p, s_TouchContext->CmToPixels(mInnerSize / 2.0f), IM_COL32(120, 120, 160, 10));
	}

	RotateButton::RotateButton(std::string name, const Vec4f& activeArea, const Actions& actions)
		: TouchControl{ std::move(name), activeArea, actions }
	{
	}

	bool RotateButton::IsHit(const Finger& finger)
	{
		if (finger.mState == Finger::State::Down &&
			mFingerID == InvalidFingerID &&
			finger.mStartPos.x >= mActiveArea.x && finger.mStartPos.x <= mActiveArea.z &&
			finger.mStartPos.y >= mActiveArea.y && finger.mStartPos.y <= mActiveArea.w)
		{
			//TFE_INFO("TOUCH", "{} is hit by finger {}", mName, finger.mID);
			return true;
		}

		return false;
	}

	void RotateButton::UpdateAction(const Finger& finger, TouchControl::State state, TouchAction& action)
	{
		if (mFingerID == finger.mID)
		{
			if (state == State::Dragged)
			{
				const Vec2f lastPos = s_TouchContext->GetAbsolutePos(finger.mLastPos);
				const Vec2f currentPos = s_TouchContext->GetAbsolutePos(finger.mCurrentPos);
				const Vec2f diff = currentPos - lastPos;
				action.mData = Vec2i{(s32) diff.x, (s32) diff.y};
			}
		}
	}

	void RotateButton::Draw()
	{
	}
}
