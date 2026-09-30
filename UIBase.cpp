#include "UIBase.h"
#include "ResourceManager.h"
#include <algorithm>

void UIBase::Init()
{
    alpha = 0;
}

void UIBase::Update(float deltaTime)
{
    finAlpha = std::clamp(finAlpha, 0, 255);
    if (deltaTime < 0.0f) deltaTime = 0.0f;
    const bool condition = displayCondition();

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



