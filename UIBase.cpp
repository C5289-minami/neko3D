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

void UIBase::Update(float deltaTime)
{
    finAlpha = std::clamp(finAlpha, 0, 255);
    if (deltaTime < 0.0f) deltaTime = 0.0f;
    position = position - shakeOffset;
    shakeOffset = {};
    const bool condition = isVisible && displayCondition();

    if (condition && (state == State::False || state == State::Hide)) {
        if (state == State::False) {
            motion.Start(0.0f, 1.0f, motion.GetDuration(), showEasing);
        } else {
            motion.SetEasing(showEasing);
        }
        state = State::Do;
    }

    if (!condition && (state == State::Normal || state == State::Do)) {
        motion.SetEasing(hideEasing);
        state = State::Hide;
    }

    if (!condition && state == State::False) return;

    if (state == State::Do) {
        motion.Increase(deltaTime);
        if (motion.IsFinished()) state = State::Normal;
    } else if (state == State::Hide) {
        motion.Decrease(deltaTime);
        if (motion.IsFinished()) state = State::False;
    }

    if (state == State::Do || state == State::Hide) {
        alpha = static_cast<int>(finAlpha * motion.GetEasedProgress());
    } else if (state == State::Normal) {
        alpha = finAlpha;
    } else {
        alpha = 0;
    }

    UpdateStartMotion();

    if (shakeElapsed < shakeDuration)
    {
        shakeElapsed = std::min(shakeElapsed + deltaTime, shakeDuration);
        const float remaining = 1.0f - shakeElapsed / shakeDuration;
        const float phase = shakeElapsed * shakeFrequency * 6.2831853f;
        shakeOffset = DxPlus::Vec2(
            std::sin(phase) * shakeIntensity * remaining,
            std::sin(phase * 1.37f) * shakeIntensity * remaining);
        position = position + shakeOffset;
    }
}

void UIBase::Draw()
{
	// Falseó‘Ô‚É‘JˆÚ‚µI‚í‚Á‚½‚ç•`‰æ‚ð’âŽ~‚·‚é
	if (state == State::False) return;
 const auto* sprite = RM().GetSprite(key);
	if (!sprite) return;
	DxLib::SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
 sprite->Draw(position, DxPlus::Vec2(scale, scale));
	DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

// •ûŒü‚ð“ü‚ê‚é‚Æo‚Ä‚­‚éMotion‚ðÝ’è‚·‚éŠÖ”
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



