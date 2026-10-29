#include "ModelAnimation.h"
#include "DxLib.h"

#include <algorithm>
#include <cmath>

#include <map>
#include <utility>

namespace
{
    // 同じモデル・同じアニメーションを識別するキー
    using AnimationKey = std::pair<int, int>;

    struct SharedAttachInfo
    {
        int attachIndex;
        int referenceCount;
    };

    // 全ModelAnimationインスタンスで共有する管理表
    std::map<AnimationKey, SharedAttachInfo> sharedAttaches;
}

// ============================================================
// AttachedAnimation
// ============================================================

ModelAnimation::AttachedAnimation::~AttachedAnimation()
{
    Reset();
}

bool ModelAnimation::AttachedAnimation::Attach(
    int modelHandle,
    int animIndex)
{
    if (modelHandle < 0 ||
        animIndex < 0 ||
        animIndex >= MV1GetAnimNum(modelHandle))
    {
        return false;
    }

    // すでに同じアニメーションを持っているなら何もしない。
    if (modelHandle_ == modelHandle &&
        animIndex_ == animIndex &&
        attachIndex_ >= 0)
    {
        return true;
    }

    Reset();

    const AnimationKey key{ modelHandle, animIndex };
    auto it = sharedAttaches.find(key);

    // ほかのModelがすでにアタッチしていたら、それを共有する。
    if (it != sharedAttaches.end())
    {
        ++it->second.referenceCount;

        modelHandle_ = modelHandle;
        animIndex_ = animIndex;
        attachIndex_ = it->second.attachIndex;

        return true;
    }

    // 初めて使う組み合わせだけアタッチする。
    const int attachIndex = MV1AttachAnim(modelHandle, animIndex);
    if (attachIndex < 0) return false;

    sharedAttaches.emplace(
        key,
        SharedAttachInfo{ attachIndex, 1 }
    );

    modelHandle_ = modelHandle;
    animIndex_ = animIndex;
    attachIndex_ = attachIndex;

    return true;
}

void ModelAnimation::AttachedAnimation::Reset()
{
    if (modelHandle_ >= 0 &&
        animIndex_ >= 0 &&
        attachIndex_ >= 0)
    {
        const AnimationKey key{ modelHandle_, animIndex_ };
        auto it = sharedAttaches.find(key);

        if (it != sharedAttaches.end() &&
            it->second.attachIndex == attachIndex_)
        {
            --it->second.referenceCount;

            // 最後に使っていたPlayerが解除した場合だけ、
            // DxLibのアタッチを解除する。
            if (it->second.referenceCount == 0)
            {
                MV1DetachAnim(modelHandle_, attachIndex_);
                sharedAttaches.erase(it);
            }
        }
    }

    modelHandle_ = -1;
    animIndex_ = -1;
    attachIndex_ = -1;
}

// ============================================================
// Play3D
// ============================================================

void ModelAnimation::Play3D(
    int modelHandle,
    int animIndex,
    bool loop,
    float speedScale,
    float blendSeconds)
{
    // 不正なモデルやアニメーション番号は無視する。
    if (modelHandle < 0 ||
        animIndex < 0 ||
        animIndex >= MV1GetAnimNum(modelHandle))
    {
        return;
    }

    speedScale = (std::max)(0.0f, speedScale);
    blendSeconds = std::isfinite(blendSeconds)
        ? (std::max)(0.0f, blendSeconds)
        : 0.0f;

    // 初回再生、またはモデルが変わった場合は直接再生する。
    if (modelHandle_ != modelHandle || !currentAnimation_)
    {
        Reset();

        if (!currentAnimation_.Attach(modelHandle, animIndex))
            return;

        modelHandle_ = modelHandle;
        currentAnimIndex_ = animIndex;
        currentElapsed_ = 0.0f;
        loop_ = loop;
        speedScale_ = speedScale;

        MV1SetAttachAnimBlendRate(
            modelHandle_,
            currentAnimation_.GetAttachIndex(),
            1.0f
        );

        SetAnimationTime(currentAnimation_, 0.0f);
        return;
    }

    loop_ = loop;
    speedScale_ = speedScale;

    // 同じ切り替え先が指定され続けても、ブレンドをやり直さない。
    if (nextAnimation_ && nextAnimIndex_ == animIndex)
        return;

    // 現在のアニメーションに戻す要求なら、ブレンドを取り消す。
    if (currentAnimIndex_ == animIndex)
    {
        if (nextAnimation_)
        {
            nextAnimation_.Reset();
            nextAnimIndex_ = -1;
            nextElapsed_ = 0.0f;
            blendElapsed_ = 0.0f;
            blendDuration_ = 0.0f;

            MV1SetAttachAnimBlendRate(
                modelHandle_,
                currentAnimation_.GetAttachIndex(),
                1.0f
            );
        }

        return;
    }

    // 切り替え時間が0秒なら、その場で切り替える。
    if (blendSeconds <= 0.0f)
    {
        nextAnimation_.Reset();
        currentAnimation_.Reset();

        if (!currentAnimation_.Attach(modelHandle, animIndex))
        {
            Reset();
            return;
        }

        currentAnimIndex_ = animIndex;
        currentElapsed_ = 0.0f;

        nextAnimIndex_ = -1;
        nextElapsed_ = 0.0f;
        blendElapsed_ = 0.0f;
        blendDuration_ = 0.0f;

        MV1SetAttachAnimBlendRate(
            modelHandle_,
            currentAnimation_.GetAttachIndex(),
            1.0f
        );

        SetAnimationTime(currentAnimation_, 0.0f);
        return;
    }

    // 別の切り替え先へ変更された場合、前のブレンドを取り消す。
    if (nextAnimation_)
    {
        nextAnimation_.Reset();
        nextAnimIndex_ = -1;
        nextElapsed_ = 0.0f;
        blendElapsed_ = 0.0f;
        blendDuration_ = 0.0f;

        MV1SetAttachAnimBlendRate(
            modelHandle_,
            currentAnimation_.GetAttachIndex(),
            1.0f
        );
    }

    // 切り替え先をアタッチする。
    if (!nextAnimation_.Attach(modelHandle, animIndex))
        return;

    nextAnimIndex_ = animIndex;
    nextElapsed_ = 0.0f;
    blendElapsed_ = 0.0f;
    blendDuration_ = blendSeconds;

    SetAnimationTime(nextAnimation_, 0.0f);

    // ブレンド開始時は現在のアニメーションだけを表示する。
    MV1SetAttachAnimBlendRate(
        modelHandle_,
        currentAnimation_.GetAttachIndex(),
        1.0f
    );

    MV1SetAttachAnimBlendRate(
        modelHandle_,
        nextAnimation_.GetAttachIndex(),
        0.0f
    );
}

// ============================================================
// Update
// ============================================================

void ModelAnimation::Update(float deltaTime)
{
    deltaTime = (std::max)(0.0f, deltaTime);

    if (!currentAnimation_) return;

    // 現在のアニメーションを進める。
    AdvanceAnimation(
        currentAnimation_,
        currentElapsed_,
        loop_,
        speedScale_,
        deltaTime
    );

    SetAnimationTime(currentAnimation_, currentElapsed_);

    if (!nextAnimation_) return;

    // 切り替え先のアニメーションも進める。
    AdvanceAnimation(
        nextAnimation_,
        nextElapsed_,
        loop_,
        speedScale_,
        deltaTime
    );

    SetAnimationTime(nextAnimation_, nextElapsed_);

    // ブレンド率を0から1へ変化させる。
    blendElapsed_ += deltaTime;

    const float blendRate = (std::min)(
        1.0f,
        blendElapsed_ / blendDuration_
        );

    MV1SetAttachAnimBlendRate(
        modelHandle_,
        currentAnimation_.GetAttachIndex(),
        1.0f - blendRate
    );

    MV1SetAttachAnimBlendRate(
        modelHandle_,
        nextAnimation_.GetAttachIndex(),
        blendRate
    );

    // ブレンド完了。切り替え先を通常の再生状態にする。
    if (blendRate >= 1.0f)
    {
        const int finishedAnimIndex = nextAnimIndex_;
        const float finishedElapsed = nextElapsed_;

        // 古いアタッチを解除してから、切り替え先を再アタッチする。
        nextAnimation_.Reset();
        currentAnimation_.Reset();

        if (!currentAnimation_.Attach(modelHandle_, finishedAnimIndex))
        {
            Reset();
            return;
        }

        currentAnimIndex_ = finishedAnimIndex;
        currentElapsed_ = finishedElapsed;

        MV1SetAttachAnimBlendRate(
            modelHandle_,
            currentAnimation_.GetAttachIndex(),
            1.0f
        );

        SetAnimationTime(currentAnimation_, currentElapsed_);

        nextAnimIndex_ = -1;
        nextElapsed_ = 0.0f;
        blendElapsed_ = 0.0f;
        blendDuration_ = 0.0f;
    }
}

// ============================================================
// Animation time
// ============================================================

void ModelAnimation::AdvanceAnimation(
    const AttachedAnimation& animation,
    float& elapsed,
    bool loop,
    float speedScale,
    float deltaTime)
{
    if (!animation) return;

    const float totalTime = MV1GetAttachAnimTotalTime(
        animation.GetModelHandle(),
        animation.GetAttachIndex()
    );

    if (totalTime <= 0.0f) return;

    elapsed += deltaTime * speedScale;

    if (loop)
    {
        elapsed = std::fmod(elapsed, totalTime);
        if (elapsed < 0.0f) elapsed += totalTime;
    }
    else
    {
        elapsed = (std::clamp)(elapsed, 0.0f, totalTime);
    }
}

void ModelAnimation::SetAnimationTime(
    const AttachedAnimation& animation,
    float elapsed)
{
    if (!animation) return;

    MV1SetAttachAnimTime(
        animation.GetModelHandle(),
        animation.GetAttachIndex(),
        elapsed
    );
}

// ============================================================
// Reset
// ============================================================

void ModelAnimation::Reset()
{
    // 次のアニメーションを先に解除する。
    nextAnimation_.Reset();
    currentAnimation_.Reset();

    modelHandle_ = -1;

    currentAnimIndex_ = -1;
    currentElapsed_ = 0.0f;

    nextAnimIndex_ = -1;
    nextElapsed_ = 0.0f;

    blendElapsed_ = 0.0f;
    blendDuration_ = 0.0f;

    loop_ = true;
    speedScale_ = 1.0f;
}