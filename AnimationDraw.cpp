#include "AnimationDraw.h"
#include "DxConv.h"

#include <algorithm>
#include <cmath>
#include <utility>

AnimationDraw::AttachedAnimation::~AttachedAnimation()
{
    Reset();
}

AnimationDraw::AttachedAnimation::AttachedAnimation(AttachedAnimation&& other) noexcept
    : modelHandle_(std::exchange(other.modelHandle_, -1)),
      attachIndex_(std::exchange(other.attachIndex_, -1))
{
}

AnimationDraw::AttachedAnimation& AnimationDraw::AttachedAnimation::operator=(AttachedAnimation&& other) noexcept
{
    if (this != &other)
    {
        Reset();
        modelHandle_ = std::exchange(other.modelHandle_, -1);
        attachIndex_ = std::exchange(other.attachIndex_, -1);
    }
    return *this;
}

bool AnimationDraw::AttachedAnimation::Attach(int modelHandle, int animIndex)
{
    Reset();
    if (modelHandle < 0 || animIndex < 0 || animIndex >= MV1GetAnimNum(modelHandle)) return false;

    const int attachIndex = MV1AttachAnim(modelHandle, animIndex);
    if (attachIndex < 0) return false;
    modelHandle_ = modelHandle;
    attachIndex_ = attachIndex;
    return true;
}

void AnimationDraw::AttachedAnimation::Reset()
{
    if (modelHandle_ >= 0 && attachIndex_ >= 0)
    {
        MV1DetachAnim(modelHandle_, attachIndex_);
    }
    modelHandle_ = -1;
    attachIndex_ = -1;
}

void AnimationDraw::Play2D(const std::vector<int>& sprites, bool loop, float animFps, float speedScale)
{
    if (mode_ != Mode::Sprite || sprites_ != sprites)
    {
        ResetModel();
        sprites_ = sprites;
        spriteElapsed_ = 0.0f;
        currentFrame_ = 0;
    }
    mode_ = Mode::Sprite;
    loop_ = loop;
    animFps_ = animFps;
    speedScale_ = speedScale;
}

void AnimationDraw::Play3D(int modelHandle, int animIndex, bool loop, float speedScale)
{
    if (mode_ != Mode::Model || modelHandle_ != modelHandle || currentAnimIndex_ != animIndex || nextAnimation_)
    {
        ResetModel();
        modelHandle_ = modelHandle;
        currentAnimIndex_ = animIndex;
        currentAnimation_.Attach(modelHandle, animIndex);
        currentElapsed_ = 0.0f;
    }
    mode_ = Mode::Model;
    loop_ = loop;
    speedScale_ = speedScale;
}

void AnimationDraw::PlayBlend3D(int modelHandle, int nextAnimIndex, float blendTime, bool loop, float speedScale)
{
    if (mode_ != Mode::Model || modelHandle_ != modelHandle || !currentAnimation_)
    {
        Play3D(modelHandle, nextAnimIndex, loop, speedScale);
        return;
    }

    if (currentAnimIndex_ == nextAnimIndex)
    {
        nextAnimation_.Reset();
        nextAnimIndex_ = -1;
        blendElapsed_ = 0.0f;
        blendDuration_ = 0.0f;
        MV1SetAttachAnimBlendRate(modelHandle_, currentAnimation_.GetAttachIndex(), 1.0f);
        loop_ = loop;
        speedScale_ = speedScale;
        return;
    }

    if (!nextAnimation_ || nextAnimIndex_ != nextAnimIndex)
    {
        nextAnimation_.Reset();
        nextAnimIndex_ = nextAnimIndex;
        nextElapsed_ = 0.0f;
        blendElapsed_ = 0.0f;
        blendDuration_ = std::max(0.0f, blendTime);
        nextAnimation_.Attach(modelHandle, nextAnimIndex);
        if (nextAnimation_)
        {
            MV1SetAttachAnimBlendRate(modelHandle, currentAnimation_.GetAttachIndex(), 1.0f);
            MV1SetAttachAnimBlendRate(modelHandle, nextAnimation_.GetAttachIndex(), 0.0f);
        }
    }
    mode_ = Mode::Model;
    loop_ = loop;
    speedScale_ = speedScale;
}

void AnimationDraw::Update(float deltaTime)
{
    deltaTime = std::max(0.0f, deltaTime);
    if (mode_ == Mode::Sprite)
    {
        if (sprites_.empty() || animFps_ <= 0.0f) return;
        const float frameDuration = 1.0f / (animFps_ * std::max(0.001f, speedScale_));
        spriteElapsed_ += deltaTime;
        while (spriteElapsed_ >= frameDuration)
        {
            spriteElapsed_ -= frameDuration;
            ++currentFrame_;
        }
        if (loop_)
        {
            currentFrame_ %= static_cast<int>(sprites_.size());
        }
        else if (currentFrame_ >= static_cast<int>(sprites_.size()))
        {
            currentFrame_ = static_cast<int>(sprites_.size()) - 1;
            spriteElapsed_ = 0.0f;
        }
        return;
    }
    if (mode_ != Mode::Model) return;

    AdvanceAnimation(currentAnimation_, currentElapsed_, loop_, speedScale_, deltaTime);
    SetAnimationTime(currentAnimation_, currentElapsed_, &currentFrame_);
    if (!nextAnimation_) return;

    AdvanceAnimation(nextAnimation_, nextElapsed_, loop_, speedScale_, deltaTime);
    SetAnimationTime(nextAnimation_, nextElapsed_);
    blendElapsed_ += deltaTime;
    const float blendRate = blendDuration_ > 0.0f ? std::min(1.0f, blendElapsed_ / blendDuration_) : 1.0f;
    MV1SetAttachAnimBlendRate(modelHandle_, currentAnimation_.GetAttachIndex(), 1.0f - blendRate);
    MV1SetAttachAnimBlendRate(modelHandle_, nextAnimation_.GetAttachIndex(), blendRate);

    if (blendRate >= 1.0f)
    {
        currentAnimation_ = std::move(nextAnimation_);
        currentAnimIndex_ = nextAnimIndex_;
        currentElapsed_ = nextElapsed_;
        nextAnimIndex_ = -1;
        nextElapsed_ = 0.0f;
        blendElapsed_ = 0.0f;
        blendDuration_ = 0.0f;
    }
}

void AnimationDraw::Draw2D(DxPlus::Vec2 pos, DxPlus::Vec2 center, DxPlus::Vec2 scale, float angle, int color) const
{
    if (mode_ != Mode::Sprite || sprites_.empty()) return;
    DxPlus::Sprite::Draw(sprites_[currentFrame_], pos, scale, center, angle, color);
}

void AnimationDraw::Draw2D(DxPlus::Vec2 pos, DxPlus::Vec2 center, float scale, float angle, int color) const
{
    Draw2D(pos, center, DxPlus::Vec2{ scale, scale }, angle, color);
}

void AnimationDraw::Draw3D(Vec3 pos, Vec3 scale, Vec3 rotation) const
{
    if (mode_ != Mode::Model || modelHandle_ < 0) return;
    MV1SetPosition(modelHandle_, DxConv::ToVECTOR(pos));
    MV1SetScale(modelHandle_, DxConv::ToVECTOR(scale));
    MV1SetRotationXYZ(modelHandle_, DxConv::ToVECTOR(rotation));
    MV1DrawModel(modelHandle_);
}

void AnimationDraw::ResetModel()
{
    currentAnimation_.Reset();
    nextAnimation_.Reset();
    modelHandle_ = -1;
    currentAnimIndex_ = -1;
    currentElapsed_ = 0.0f;
    nextAnimIndex_ = -1;
    nextElapsed_ = 0.0f;
    blendElapsed_ = 0.0f;
    blendDuration_ = 0.0f;
}

void AnimationDraw::Reset()
{
    ResetModel();
    sprites_.clear();
    mode_ = Mode::None;
    spriteElapsed_ = 0.0f;
    currentFrame_ = 0;
}

void AnimationDraw::AdvanceAnimation(const AttachedAnimation& animation, float& elapsed, bool loop, float speedScale, float deltaTime)
{
    if (!animation) return;
    const float totalTime = MV1GetAttachAnimTotalTime(animation.GetModelHandle(), animation.GetAttachIndex());
    if (totalTime <= 0.0f) return;
    elapsed += deltaTime * speedScale;
    if (loop)
    {
        elapsed = std::fmod(elapsed, totalTime);
        if (elapsed < 0.0f) elapsed += totalTime;
    }
    else
    {
        elapsed = std::clamp(elapsed, 0.0f, totalTime);
    }
}

void AnimationDraw::SetAnimationTime(const AttachedAnimation& animation, float elapsed, int* currentFrame)
{
    if (!animation) return;
    const int modelHandle = animation.GetModelHandle();
    const int attachIndex = animation.GetAttachIndex();
    const float totalTime = MV1GetAttachAnimTotalTime(modelHandle, attachIndex);
    if (totalTime <= 0.0f) return;
    MV1SetAttachAnimTime(modelHandle, attachIndex, elapsed);
    if (currentFrame) *currentFrame = static_cast<int>((elapsed / totalTime) * 100.0f);
}