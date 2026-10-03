#pragma once

#include "DxPlus/DxPlus.h"
#include "Vector3.h"
#include <vector>

class AnimationDraw {
public:
    AnimationDraw() = default;
    ~AnimationDraw() = default;

    AnimationDraw(const AnimationDraw&) = delete;
    AnimationDraw& operator=(const AnimationDraw&) = delete;

    void Play2D(const std::vector<int>& sprites, bool loop, float animFps = 30.0f, float speedScale = 1.0f);
    void Play3D(int modelHandle, int animIndex, bool loop, float speedScale = 1.0f);
    void PlayBlend3D(int modelHandle, int nextAnimIndex, float blendTime, bool loop, float speedScale = 1.0f);
    void Update(float deltaTime);

    void Draw2D(DxPlus::Vec2 pos, DxPlus::Vec2 center = { 0.0f, 0.0f }, DxPlus::Vec2 scale = { 1.0f, 1.0f }, float angle = 0.0f, int color = GetColor(255, 255, 255)) const;
    void Draw2D(DxPlus::Vec2 pos, DxPlus::Vec2 center, float scale, float angle = 0.0f, int color = GetColor(255, 255, 255)) const;
    void Draw3D(Vec3 pos, Vec3 scale = { 1.0f, 1.0f, 1.0f }, Vec3 rotation = { 0.0f, 0.0f, 0.0f }) const;

    int GetCurrentFrame() const { return currentFrame_; }
    void Reset();

private:
    class AttachedAnimation {
    public:
        AttachedAnimation() = default;
        ~AttachedAnimation();
        AttachedAnimation(const AttachedAnimation&) = delete;
        AttachedAnimation& operator=(const AttachedAnimation&) = delete;
        AttachedAnimation(AttachedAnimation&& other) noexcept;
        AttachedAnimation& operator=(AttachedAnimation&& other) noexcept;

        bool Attach(int modelHandle, int animIndex);
        void Reset();
        explicit operator bool() const { return attachIndex_ >= 0; }
        int GetModelHandle() const { return modelHandle_; }
        int GetAttachIndex() const { return attachIndex_; }

    private:
        int modelHandle_{ -1 };
        int attachIndex_{ -1 };
    };

    enum class Mode { None, Sprite, Model };

    static void AdvanceAnimation(const AttachedAnimation& animation, float& elapsed, bool loop, float speedScale, float deltaTime);
    static void SetAnimationTime(const AttachedAnimation& animation, float elapsed, int* currentFrame = nullptr);
    void ResetModel();

    Mode mode_{ Mode::None };
    std::vector<int> sprites_;
    bool loop_{ true };
    float animFps_{ 30.0f };
    float speedScale_{ 1.0f };
    float spriteElapsed_{ 0.0f };
    int currentFrame_{ 0 };

    int modelHandle_{ -1 };
    int currentAnimIndex_{ -1 };
    AttachedAnimation currentAnimation_;
    float currentElapsed_{ 0.0f };
    int nextAnimIndex_{ -1 };
    AttachedAnimation nextAnimation_;
    float nextElapsed_{ 0.0f };
    float blendElapsed_{ 0.0f };
    float blendDuration_{ 0.0f };
};