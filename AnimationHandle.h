#pragma once

#include "DxLib.h"

class AnimationHandle
{
public:
    AnimationHandle() = default;

    // アニメーション名からハンドルを取得する。
    static AnimationHandle Find(
        int sourceModelHandle,
        const wchar_t* animationName)
    {
        if (sourceModelHandle < 0 || animationName == nullptr)
            return {};

        const int animNum = MV1GetAnimNum(sourceModelHandle);
        if (animNum <= 0)
            return {};

        const int animIndex = MV1GetAnimIndex(
            sourceModelHandle,
            animationName
        );

        if (animIndex < 0 || animIndex >= animNum)
            return {};

        return AnimationHandle(sourceModelHandle, animIndex);
    }
    // アニメーション番号からハンドルを取得する。
    static AnimationHandle FromIndex(
        int sourceModelHandle,
        int animIndex)
    {
        if (sourceModelHandle < 0)
            return {};

        const int animNum = MV1GetAnimNum(sourceModelHandle);

        if (animIndex < 0 || animIndex >= animNum)
            return {};

        return AnimationHandle(sourceModelHandle, animIndex);
    }

    explicit operator bool() const
    {
        return sourceModelHandle_ >= 0 && animIndex_ >= 0;
    }

    bool operator==(const AnimationHandle& other) const
    {
        return sourceModelHandle_ == other.sourceModelHandle_ &&
            animIndex_ == other.animIndex_;
    }

    bool operator!=(const AnimationHandle& other) const
    {
        return !(*this == other);
    }

    int GetSourceModelHandle() const { return sourceModelHandle_; }
    int GetAnimIndex() const { return animIndex_; }

private:
    AnimationHandle(int sourceModelHandle, int animIndex)
        : sourceModelHandle_(sourceModelHandle),
        animIndex_(animIndex)
    {
    }

    int sourceModelHandle_{ -1 };
    int animIndex_{ -1 };
};