#include "TwoFrameUIAnimator.h"

#include "UIBase.h"

#include <algorithm>
#include <cmath>

void TwoFrameUIAnimator::SetUI(
    UIBase* firstUI,
    UIBase* secondUI)
{
    firstUI_ = firstUI;
    secondUI_ = secondUI;
}

void TwoFrameUIAnimator::SetAnchorPosition(
    const DxPlus::Vec2& position)
{
    anchorPosition_ = position;

    if (!playing_)
    {
        return;
    }

    switch (phase_)
    {
    case Phase::FirstHold:
        ApplyFirstTransform();
        break;

    case Phase::SecondHold:
        ApplySecondTransform();
        break;

    case Phase::ToSecond:
    case Phase::ToFirst:
        break;
    }
}

void TwoFrameUIAnimator::SetFirstFrame(
    const FrameSetting& setting)
{
    firstFrame_ = setting;
}

void TwoFrameUIAnimator::SetSecondFrame(
    const FrameSetting& setting)
{
    secondFrame_ = setting;
}

void TwoFrameUIAnimator::SetFirstHoldDuration(
    float duration)
{
    if (!std::isfinite(duration))
    {
        return;
    }

    firstHoldDuration_ =
        (std::max)(0.0f, duration);
}

void TwoFrameUIAnimator::SetSecondHoldDuration(
    float duration)
{
    if (!std::isfinite(duration))
    {
        return;
    }

    secondHoldDuration_ =
        (std::max)(0.0f, duration);
}

void TwoFrameUIAnimator::SetForwardDuration(
    float duration)
{
    if (!std::isfinite(duration))
    {
        return;
    }

    forwardDuration_ =
        (std::max)(0.0f, duration);
}

void TwoFrameUIAnimator::SetBackwardDuration(
    float duration)
{
    if (!std::isfinite(duration))
    {
        return;
    }

    backwardDuration_ =
        (std::max)(0.0f, duration);
}

void TwoFrameUIAnimator::Start()
{
    if (!firstUI_ || !secondUI_)
    {
        return;
    }

    playing_ = true;

    firstUI_->SetTrue();
    secondUI_->SetFalse();

    ApplyFirstTransform();
    ApplySecondTransform();

    ChangePhase(Phase::FirstHold);
}

void TwoFrameUIAnimator::Stop()
{
    playing_ = false;
    phaseElapsed_ = 0.0f;
    motion_.Reset();

    if (firstUI_)
    {
        firstUI_->SetFalse();
    }

    if (secondUI_)
    {
        secondUI_->SetFalse();
    }
}

void TwoFrameUIAnimator::Update(float deltaTime)
{
    if (!playing_ ||
        !firstUI_ ||
        !secondUI_ ||
        !std::isfinite(deltaTime))
    {
        return;
    }

    deltaTime = (std::max)(0.0f, deltaTime);

    switch (phase_)
    {
    case Phase::FirstHold:
    {
        phaseElapsed_ += deltaTime;

        if (phaseElapsed_ >= firstHoldDuration_)
        {
            ChangePhase(Phase::ToSecond);
        }

        break;
    }

    case Phase::ToSecond:
    {
        motion_.Update(deltaTime);

        ApplyInterpolatedTransform(
            *firstUI_,
            firstFrame_,
            secondFrame_,
            motion_.GetEasedProgress()
        );

        if (motion_.IsFinished())
        {
            // 同じ位置・角度で画像を切り替える。
            ApplySecondTransform();

            firstUI_->SetFalse();
            secondUI_->SetTrue();

            ChangePhase(Phase::SecondHold);
        }

        break;
    }

    case Phase::SecondHold:
    {
        phaseElapsed_ += deltaTime;

        if (phaseElapsed_ >= secondHoldDuration_)
        {
            ChangePhase(Phase::ToFirst);
        }

        break;
    }

    case Phase::ToFirst:
    {
        motion_.Update(deltaTime);

        ApplyInterpolatedTransform(
            *secondUI_,
            secondFrame_,
            firstFrame_,
            motion_.GetEasedProgress()
        );

        if (motion_.IsFinished())
        {
            // 同じ位置・角度で画像を切り替える。
            ApplyFirstTransform();

            secondUI_->SetFalse();
            firstUI_->SetTrue();

            ChangePhase(Phase::FirstHold);
        }

        break;
    }
    }
}

void TwoFrameUIAnimator::ChangePhase(Phase phase)
{
    phase_ = phase;
    phaseElapsed_ = 0.0f;

    switch (phase_)
    {
    case Phase::FirstHold:
        ApplyFirstTransform();
        break;

    case Phase::ToSecond:
        if (forwardDuration_ <= 0.0f)
        {
            ApplySecondTransform();

            firstUI_->SetFalse();
            secondUI_->SetTrue();

            ChangePhase(Phase::SecondHold);
            return;
        }

        motion_.Start(
            0.0f,
            1.0f,
            forwardDuration_,
            Motion::Easing::EaseOut
        );
        break;

    case Phase::SecondHold:
        ApplySecondTransform();
        break;

    case Phase::ToFirst:
        if (backwardDuration_ <= 0.0f)
        {
            ApplyFirstTransform();

            secondUI_->SetFalse();
            firstUI_->SetTrue();

            ChangePhase(Phase::FirstHold);
            return;
        }

        motion_.Start(
            0.0f,
            1.0f,
            backwardDuration_,
            Motion::Easing::EaseIn
        );
        break;
    }
}

void TwoFrameUIAnimator::ApplyFirstTransform()
{
    if (!firstUI_)
    {
        return;
    }

    ApplyTransform(
        *firstUI_,
        firstFrame_
    );
}

void TwoFrameUIAnimator::ApplySecondTransform()
{
    if (!secondUI_)
    {
        return;
    }

    ApplyTransform(
        *secondUI_,
        secondFrame_
    );
}

void TwoFrameUIAnimator::ApplyTransform(
    UIBase& ui,
    const FrameSetting& setting)
{
    ui.SetPosition({
        anchorPosition_.x + setting.offset.x,
        anchorPosition_.y + setting.offset.y
        });

    ui.SetAngle(setting.angle);
    ui.SetScale(setting.scale);
    ui.SetCenter(setting.center);
    ui.SetFlipX(setting.flipX);
}

void TwoFrameUIAnimator::ApplyInterpolatedTransform(
    UIBase& ui,
    const FrameSetting& from,
    const FrameSetting& to,
    float progress)
{
    FrameSetting setting;

    setting.offset = Lerp(
        from.offset,
        to.offset,
        progress
    );

    setting.angle = Lerp(
        from.angle,
        to.angle,
        progress
    );

    setting.scale = Lerp(
        from.scale,
        to.scale,
        progress
    );

    setting.center = from.center;
    setting.flipX = from.flipX;

    ApplyTransform(
        ui,
        setting
    );
}

float TwoFrameUIAnimator::Lerp(
    float from,
    float to,
    float progress)
{
    progress = (std::clamp)(
        progress,
        0.0f,
        1.0f
        );

    return from +
        (to - from) * progress;
}

DxPlus::Vec2 TwoFrameUIAnimator::Lerp(
    const DxPlus::Vec2& from,
    const DxPlus::Vec2& to,
    float progress)
{
    progress = (std::clamp)(
        progress,
        0.0f,
        1.0f
        );

    return {
        Lerp(from.x, to.x, progress),
        Lerp(from.y, to.y, progress)
    };
}