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
    if (state == State::False || state == State::Hide)
        return;

    isVisible = false;
    fadeOnlyMotion = false;

    motion.Start(0.0f, 1.0f, motion.GetDuration(), hideEasing);
    motion.StartReverse();

    state = State::Hide;
}

void UIBase::Update(float deltaTime)
{
    deltaTime = std::max(0.0f, deltaTime);
    finAlpha = std::clamp(finAlpha, 0, 255);

    position = position - shakeOffset;
    shakeOffset = {};

    const bool shouldShow = isVisible && displayCondition();

    // 表示開始
    if (shouldShow && state == State::False)
    {
        motion.Start(0.0f, 1.0f, motion.GetDuration(), showEasing);
        state = State::Do;
    }

    // 表示条件が無効になったら非表示モーションへ
    if (!shouldShow && (state == State::Normal || state == State::Do))
    {
        motion.SetEasing(hideEasing);
        state = State::Hide;
    }

    // モーション更新
    switch (state)
    {
    case State::Do:
        motion.Increase(deltaTime);
        if (motion.IsFinished())
            state = State::Normal;
        break;

    case State::Hide:
        motion.Decrease(deltaTime);
        if (motion.IsFinished())
            state = State::False;
        break;

    case State::Normal:
    case State::False:
        break;
    }

    // 透明度更新
    if (state == State::Do || state == State::Hide)
        alpha = static_cast<int>(finAlpha * motion.GetEasedProgress());
    else
        alpha = (state == State::Normal) ? finAlpha : 0;

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
	// False状態に遷移し終わったら描画を停止する
	if (state == State::False) return;
 const auto* sprite = RM().GetSprite(key);
	if (!sprite) return;
	DxLib::SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
 sprite->Draw(position, DxPlus::Vec2(scale, scale));
	DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

// 方向を入れると出てくるMotionを設定する関数
void UIBase::SetToMove(DxPlus::Vec2 dir)
{
    state = State::Do;
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
        if (state == State::Do) {
            position = (basePosition - Direction * MoveDistance) + Direction * (MoveDistance * easedProgress);
        }
        else if (state == State::Hide) {
            position = basePosition - Direction * (MoveDistance * (1.0f - easedProgress));
        }
    }
}



