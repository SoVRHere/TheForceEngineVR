#include "touchInput.h"
#include <TFE_System/math.h>
#include <TFE_System/system.h>
#include <TFE_Ui/imGUI/imgui.h>
#include <vector>

namespace TFE_Input
{
	class TouchContext
	{
	public:
		TouchContext(bool enable);

	public:
		Vec2f GetAbsolutePos(const Vec2f& rel) const;
		float PixelsToCm(float pixels) const;
		Vec2f PixelsToCm(const Vec2f& pixels) const;
		float CmToPixels(float cms) const;
		const Vec2f& GetDisplaySize() const { return mDisplaySize; }

		void HandleEvents(const SDL_Event& event);

		bool mEnabled{ false };
		float mDpi = 400.0f;  // ~ fullhd on 6"
		Vec2f mDisplaySize;

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
			break;
		case Action::RightMouseButtonUp:
			break;
		case Action::MiddleMouseButtonDown:
			break;
		case Action::MiddleMouseButtonUp:
			break;
		// controller
		case Action::LeftAxis:
			break;
		case Action::RightAxis:
			break;
		case Action::LeftTrigger:
			break;
		case Action::RightTrigger:
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
		default:
			TFE_ERROR("TOUCH", "action {} not handled", EnumValue(action.mAction));
			break;
		}
	}

	void initTouchControls(bool enable)
	{
		s_TouchContext = std::make_unique<TouchContext>(enable);

//		std::unordered_map<Finger::State, TouchAction> actions = {
//			{ Finger::State::Down, TouchAction{ TouchAction::Action::LeftMouseButtonDown, std::monostate{} } },
//			{ Finger::State::Up, TouchAction{ TouchAction::Action::LeftMouseButtonUp, std::monostate{} } }
//		};
		auto fireControl = std::make_unique<CircleButton>("btn_fire", Vec4f{ 0.0f, 0.0f, 1.0f, 1.0f }, Vec2f{ 0.8f, 0.8f }, 2.0f,
			TouchControl::Actions {
				{ TouchControl::State::Clicked, TouchAction{ TouchAction::Action::LeftMouseButtonDown, std::monostate{} } },
				{ TouchControl::State::Dragged, TouchAction{ TouchAction::Action::LeftMouseButtonDown, std::monostate{} } },
				{ TouchControl::State::Released, TouchAction{ TouchAction::Action::LeftMouseButtonUp, std::monostate{} } }
			});
		s_TouchControls.push_back(std::move(fireControl));

		auto jumpControl = std::make_unique<CircleButton>("btn_jump", Vec4f{ 0.0f, 0.0f, 1.0f, 1.0f }, Vec2f{ 0.8f, 0.4f }, 2.0f,
			TouchControl::Actions {
				{ TouchControl::State::Clicked, TouchAction{ TouchAction::Action::AButtonDown, std::monostate{} } },
				{ TouchControl::State::Released, TouchAction{ TouchAction::Action::AButtonUp, std::monostate{} } }
			});
		s_TouchControls.push_back(std::move(jumpControl));
	}

	void destroyTouchControls()
	{
		s_TouchControls.clear();
		s_TouchContext.reset();
	}


	void handleTouchEvents(const SDL_Event& event)
	{
		if (s_TouchContext && s_TouchContext->mEnabled)
		{
			s_TouchContext->HandleEvents(event);
		}
	}

	void drawTouchControls()
	{
		if (!s_TouchContext || !s_TouchContext->mEnabled)
		{
			return;
		}

		ImGuiIO& io = ImGui::GetIO();
		s_TouchContext->mDisplaySize = io.DisplaySize;

		const uint32_t windowFlags = ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground;
		//ImGui::PushFont(fpsFont);
		ImGui::SetNextWindowSize(s_TouchContext->mDisplaySize);
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
	{
		float dpi = 400.0f;  // ~ fullhd on 6"
		SDL_GetDisplayDPI(0, nullptr, &dpi, nullptr);

		TFE_INFO("TOUCH", "DPI={}", dpi);
	}

	Vec2f TouchContext::GetAbsolutePos(const Vec2f& rel) const
	{
		return mDisplaySize * rel;
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

	void TouchContext::HandleEvents(const SDL_Event& event)
	{
		if (!mEnabled)
		{
			return;
		}

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
				Finger newFinger{ finger.fingerId, Finger::State::Down, pos, pos };
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
						TFE_INFO("TOUCH", "{} is clicked by finger {}", mName, mFingerID);
					}
				}
			} break;
			case Finger::State::Motion:
			{
				if (mFingerID == finger.mID)
				{
					mState = State::Dragged;
					TFE_INFO("TOUCH", "{} is dragged by finger {}", mName, mFingerID);
				}
				else if (mFingerID == InvalidFingerID)
				{
					if (IsHit(finger))
					{
						mState = State::Hovered;
						TFE_INFO("TOUCH", "{} is hovered by finger {}", mName, finger.mID);
					}
					else if (mState == State::Hovered)
					{
						mState = State::None;
						TFE_INFO("TOUCH", "{} is unhovered by finger {}", mName, finger.mID);
					}
				}
			} break;
			case Finger::State::Up:
			{
				if (mFingerID == finger.mID)
				{
					mState = State::Released;
					mFingerID = InvalidFingerID;
					TFE_INFO("TOUCH", "{} is released by finger {}", mName, finger.mID);
				}
				else if (mState == State::Hovered)
				{
					mState = State::Unhovered;
					TFE_INFO("TOUCH", "{} is unhovered by finger {}", mName, finger.mID);
				}
			} break;
			default:
				TFE_ERROR("TOUCH", "{}, finger state {} not handled", mName, EnumValue(finger.mState));
				break;
		}

		if (auto it = mActions.find(mState); it != mActions.end())
		{
			TouchAction::Perform(it->second);
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

//	bool CircleButton::HandleEvent(const Finger& finger)
//	{
//		//TFE_INFO("TOUCH", "{} CircleButton::HandleEvent", mName);
//		bool hit = IsHit(finger);
//
//		if (mFingerID == InvalidFingerID)
//		{
//			//finger.state == Finger::State::
//		}
//		return true;
//	}

	bool DrawCircleButton(const char* label, const Vec2f& center, float radius, ImU32 color, ImU32 hoverColor)
	{
		ImGui::GetWindowDrawList()->AddCircleFilled(center, radius, color, 32);

		ImGui::SetCursorScreenPos(center - radius);
		ImGui::InvisibleButton(label, ImVec2(2 * radius, 2 * radius));

		bool hovered = ImGui::IsItemHovered();
		bool clicked = ImGui::IsItemClicked();

		if (hovered)
		{
			ImGui::GetWindowDrawList()->AddCircleFilled(center, radius, hoverColor, 32);
		}

		return clicked;
	}

	void CircleButton::Draw()
	{
		Vec2f p = s_TouchContext->GetAbsolutePos(mPos);
		DrawCircleButton(mName.c_str(), p, s_TouchContext->CmToPixels(mSize / 2.0f), IM_COL32(120, 120, 160, 60), IM_COL32(0, 255, 0, 255));
	}

//	CircleButton::~CircleButton()
//	{
//
//	}
}
