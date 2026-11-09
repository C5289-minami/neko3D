#pragma once

#include "DxPlus/DxPlus.h"
#include "Motion.h"

class UIBase;

class TwoFrameUIAnimator
{
public:
	struct FrameSetting
	{
		DxPlus::Vec2 offset{};

		float angle{};

		float scale{ 1.0f };

		DxPlus::Vec2 center{
			0.5f,
			0.5f
		};

		bool flipX{ false };
	};

	TwoFrameUIAnimator() = default;

	void SetUI(
		UIBase* firstUI,
		UIBase* secondUI
	);

	void SetAnchorPosition(
		const DxPlus::Vec2& position
	);

	void SetFirstFrame(
		const FrameSetting& setting
	);

	void SetSecondFrame(
		const FrameSetting& setting
	);

	void SetFirstHoldDuration(float duration);
	void SetSecondHoldDuration(float duration);

	void SetForwardDuration(float duration);
	void SetBackwardDuration(float duration);

	void Start();
	void Stop();
	void Update(float deltaTime);

	bool IsPlaying() const
	{
		return playing_;
	}
	UIBase* GetFirstUI() const
	{
		return firstUI_;
	}

	UIBase* GetSecondUI() const
	{
		return secondUI_;
	}
	void Pause() {
		playing_ = false;
	}

private:
	enum class Phase
	{
		FirstHold,
		ToSecond,
		SecondHold,
		ToFirst
	};

	void ChangePhase(Phase phase);

	void ApplyFirstTransform();
	void ApplySecondTransform();

	void ApplyTransform(
		UIBase& ui,
		const FrameSetting& setting
	);

	void ApplyInterpolatedTransform(
		UIBase& ui,
		const FrameSetting& from,
		const FrameSetting& to,
		float progress
	);

	static float Lerp(
		float from,
		float to,
		float progress
	);

	static DxPlus::Vec2 Lerp(
		const DxPlus::Vec2& from,
		const DxPlus::Vec2& to,
		float progress
	);

private:
	UIBase* firstUI_{};
	UIBase* secondUI_{};

	DxPlus::Vec2 anchorPosition_{};

	FrameSetting firstFrame_{};
	FrameSetting secondFrame_{};

	Motion motion_;

	Phase phase_{ Phase::FirstHold };

	float phaseElapsed_{};

	float firstHoldDuration_{ 0.6f };
	float secondHoldDuration_{ 0.15f };

	float forwardDuration_{ 0.15f };
	float backwardDuration_{ 0.2f };

	bool playing_{};
};