#include "UIBase.h"
#include "ResourceManager.h"
#include <algorithm>
#include <cmath>

void UIBase::Init()
{
	alpha = 0;
}

void UIBase::FadeOut(float duration)
{
	if (duration >= 0.0f) SetMotionDuration(duration);
	fadeOnlyMotion = true;
	isVisible = false;
}

void UIBase::FadeIn(float duration)
{
	if (duration >= 0.0f) SetMotionDuration(duration);
	fadeOnlyMotion = true;
	isVisible = true;
}

void UIBase::Shake(float intensity, float duration, float frequency)
{
	shakeIntensity = std::max(0.0f, intensity);
	shakeDuration = std::max(0.0f, duration);
	shakeFrequency = std::max(0.0f, frequency);
	shakeElapsed = 0.0f;
	shakeOffset = {};
}

void UIBase::Hide()
{
	if (state == UIState::False || state == UIState::Hide)
		return;

	isVisible = false;
	fadeOnlyMotion = false;

	motion.Start(0.0f, 1.0f, motion.GetDuration(), hideEasing);
	motion.StartReverse();

	state = UIState::Hide;
}

void UIBase::Update(float deltaTime)
{
	deltaTime = std::max(0.0f, deltaTime);
	finAlpha = std::clamp(finAlpha, 0, 255);
	angle += angleSpeed * deltaTime;

	position = position - shakeOffset;
	shakeOffset = {};

	const bool shouldShow = isVisible && displayCondition();

	// 表示開始
	if (shouldShow && state == UIState::False)
	{
		motion.Start(0.0f, 1.0f, motion.GetDuration(), showEasing);
		state = UIState::Do;
	}

	// 表示条件が無効になったら非表示モーションへ
	if (!shouldShow && (state == UIState::Normal || state == UIState::Do))
	{
		motion.SetEasing(hideEasing);
		state = UIState::Hide;
	}

	// モーション更新
	switch (state)
	{
	case UIState::Do:
		motion.Increase(deltaTime);
		if (motion.IsFinished())
			state = UIState::Normal;
		break;

	case UIState::Hide:
		motion.Decrease(deltaTime);
		if (motion.IsFinished())
			state = UIState::False;
		break;

	case UIState::Normal:
	case UIState::False:
		break;
	}

	// 透明度更新
	if (!manualAlpha)
	{
		if (state == UIState::Do || state == UIState::Hide)
			alpha = static_cast<int>(finAlpha * motion.GetEasedProgress());
		else
			alpha = (state == UIState::Normal) ? finAlpha : 0;
	}

	UpdateStartMotion();

	// 揺れ
	if (shakeElapsed < shakeDuration)
	{
		shakeElapsed = std::min(shakeElapsed + deltaTime, shakeDuration);

		const float remaining = 1.0f - shakeElapsed / shakeDuration;
		const float phase = shakeElapsed * shakeFrequency * 6.2831853f;

		shakeOffset = DxPlus::Vec2(
			std::sin(phase) * shakeIntensity * remaining,
			std::sin(phase * 1.37f) * shakeIntensity * remaining
		);

		position = position + shakeOffset;
	}
}

void UIBase::Draw()
{
	if (state == UIState::False) return;

	const auto* sprite = RM().GetSprite(key);
	if (!sprite) return;

	float width = 0.0f;
	float height = 0.0f;
	DxLib::GetGraphSizeF(sprite->GetID(), &width, &height);

	DxPlus::Vec2 drawPosition(
		position.x + (0.5f - center.x) * width * scale,
		position.y + (0.5f - center.y) * height * scale
	);

	DxLib::SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);


	{
		const float drawCenterX =
			position.x + (0.5f - center.x) * width;

		const float drawCenterY =
			position.y + (0.5f - center.y) * height;

		DxLib::DrawRotaGraph(
			static_cast<int>(drawCenterX),
			static_cast<int>(drawCenterY),
			scale,
			angle,
			sprite->GetID(),
			TRUE
		);
	}


	DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

// 方向を入れると出てくるMotionを設定する関数
void UIBase::SetToMove(DxPlus::Vec2 dir)
{
	state = UIState::Do;
	motion.Start(0.0f, 1.0f, motion.GetDuration(), showEasing);
	Direction = dir;
	if (motion.GetDuration() > 0)
		position = basePosition - Direction * MoveDistance;
}

void UIBase::UpdateStartMotion()
{
	if (fadeOnlyMotion)
	{
		position = basePosition;
		return;
	}

	if (motion.GetDuration() > 0.0f) {
		const float easedProgress = motion.GetEasedProgress();
		if (state == UIState::Do) {
			position = (basePosition - Direction * MoveDistance) + Direction * (MoveDistance * easedProgress);
		}
		else if (state == UIState::Hide) {
			position = basePosition - Direction * (MoveDistance * (1.0f - easedProgress));
		}
	}
}



