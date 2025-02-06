#pragma once

#include <TFE_System/types.h>
#include <TFE_Input/input.h>
#include <SDL.h>
#include <unordered_map>
#include <variant>
#include <string>

namespace TFE_Input
{
	void initTouchControls(bool enable);
	void destroyTouchControls();
	void handleTouchEvents(const SDL_Event& event);
	void drawTouchControls();

	using FingerID = SDL_FingerID;
	constexpr FingerID InvalidFingerID = -9999;

	struct Finger
	{
		enum class State
		{
			Down,
			Up,
			Motion
		};

		FingerID mID;
		State mState;
		Vec2f mStartPos;
		Vec2f mCurrentPos;
	};

	struct TouchAction
	{
		enum class Action
		{
			// mouse
			LeftMouseButtonDown,
			LeftMouseButtonUp,
			RightMouseButtonDown,
			RightMouseButtonUp,
			MiddleMouseButtonDown,
			MiddleMouseButtonUp,
			// controller
			LeftAxis,
			RightAxis,
			LeftTrigger,
			RightTrigger,
			AButtonDown,
			AButtonUp,
			BButtonDown,
			BButtonUp,
			XButtonDown,
			XButtonUp,
			YButtonDown,
			YButtonUp
		};
		using Data = std::variant<std::monostate, bool, float, Vec2f>;

		Action mAction;
		Data mData;

		static void Perform(const TouchAction& action);
	};

	class TouchControl
	{
	public:
		enum class State
		{
			None,
			Clicked,
			Dragged,
			Hovered,
			Unhovered,
			Released
		};

		using Actions = std::unordered_map<TouchControl::State, TouchAction>;

	public:
		TouchControl(std::string name, const Vec4f& activeArea, const Actions& actions);

	public:
		virtual bool HandleEvent(const Finger& finger);
		virtual bool IsHit(const Finger& finger) = 0;
		virtual void Draw() = 0;
		virtual ~TouchControl() = default;

	public:
		FingerID GetFingerID() const { return mFingerID; }
		State GetState() const { return mState; }

	protected:
		std::string mName;
		Vec4f		mActiveArea;
		Actions		mActions;
		State 		mState{ State::None };
		FingerID 	mFingerID{ InvalidFingerID };
	};

	class CircleButton : public TouchControl
	{
	public:
		CircleButton(std::string name, const Vec4f& activeArea, const Vec2f& pos, float size, const Actions& actions);

	public:
		virtual bool IsHit(const Finger& finger) override;
		//virtual bool HandleEvent(const Finger& finger) override;
		virtual void Draw() override;
		//virtual ~CircleButton();

	private:
		Vec2f		mPos;
		float		mSize;
	};
}
