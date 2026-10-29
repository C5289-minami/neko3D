#pragma once

class ModelAnimation
{
public:
    ModelAnimation() = default;
    ~ModelAnimation() = default;

    ModelAnimation(const ModelAnimation&) = delete;
    ModelAnimation& operator=(const ModelAnimation&) = delete;

    // アニメーションを再生する。
    // 前回と異なるアニメーションなら blendSeconds 秒かけて切り替える。
    void Play3D(
        int modelHandle,
        int animIndex,
        bool loop,
        float speedScale = 1.0f,
        float blendSeconds = 0.15f
    );

    // 再生時間とブレンドを更新する。
    void Update(float deltaTime);

    // アニメーションを解除する。
    void Reset();

    int GetCurrentAnimIndex() const { return currentAnimIndex_; }
    bool IsBlending() const { return static_cast<bool>(nextAnimation_); }

private:
    class AttachedAnimation
    {
    public:
        AttachedAnimation() = default;
        ~AttachedAnimation();

        AttachedAnimation(const AttachedAnimation&) = delete;
        AttachedAnimation& operator=(const AttachedAnimation&) = delete;

        bool Attach(int modelHandle, int animIndex);
        void Reset();

        explicit operator bool() const { return attachIndex_ >= 0; }

        int GetModelHandle() const { return modelHandle_; }
        int GetAttachIndex() const { return attachIndex_; }

    private:
        int modelHandle_{ -1 };
        int animIndex_{ -1 };
        int attachIndex_{ -1 };
    };

    static void AdvanceAnimation(
        const AttachedAnimation& animation,
        float& elapsed,
        bool loop,
        float speedScale,
        float deltaTime
    );

    static void SetAnimationTime(
        const AttachedAnimation& animation,
        float elapsed
    );

    int modelHandle_{ -1 };

    // 現在再生しているアニメーション
    AttachedAnimation currentAnimation_;
    int currentAnimIndex_{ -1 };
    float currentElapsed_{ 0.0f };

    // 切り替え先のアニメーション
    AttachedAnimation nextAnimation_;
    int nextAnimIndex_{ -1 };
    float nextElapsed_{ 0.0f };

    // ブレンドの進行状況
    float blendElapsed_{ 0.0f };
    float blendDuration_{ 0.0f };

    bool loop_{ true };
    float speedScale_{ 1.0f };
};
