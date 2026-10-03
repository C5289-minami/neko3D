#include "AnimationDraw.h"
#include "AnimationDraw.h"
#include "DxConv.h"

#include <algorithm>
#include <cmath>

// ============================================================================
// フレーム更新の共通処理
// ============================================================================
void AnimationDraw::UpdateFrameInternal(float speed, int spriteCount, bool loop)
{
    if (spriteCount == 0) return;

    spriteNum = spriteCount;
    frameInterval = speed * 60.0f;

    currentInterval += 1.0f;

    if (currentInterval >= frameInterval)
    {
        currentframe++;
        currentInterval = 0.0f;
    }

    if (loop)
    {
        if (currentframe >= spriteNum)
        {
            currentframe = 0;
        }
    }
    else
    {
        if (currentframe >= spriteNum)
        {
            currentframe = spriteNum - 1;
        }
    }
}

// ============================================================================
// 2D描画（スプライト版・スケールが(x, y)）
// ============================================================================
void AnimationDraw::DrawAnim(
    DxPlus::Vec2 pos,
    const std::vector<int>& sprite,
    bool loop,
    float animSpeed,
    DxPlus::Vec2 center,
    DxPlus::Vec2 scale,
    float angle,
    int color
)
{
    position = pos;
    AnimationSpeed = animSpeed;

    if (sprite.empty()) return;

    UpdateFrameInternal(animSpeed, static_cast<int>(sprite.size()), loop);

    // スプライトを描画
    DxPlus::Sprite::Draw(sprite[currentframe], position, scale, center, angle, color);
}

// ============================================================================
// 2D描画（スプライト版・スケールが単一値）
// ============================================================================
void AnimationDraw::DrawAnim(
    DxPlus::Vec2 pos,
    const std::vector<int>& sprite,
    bool loop,
    float animSpeed,
    DxPlus::Vec2 center,
    float scale,
    float angle,
    int color
)
{
    DrawAnim(pos, sprite, loop, animSpeed, center, DxPlus::Vec2{ scale, scale }, angle, color);
}

// ============================================================================
// 3D描画（MV1モデル版）
// ============================================================================
void AnimationDraw::DrawAnim3D(
    int modelHandle,
    Vec3 pos,
    int animIndex,
    bool loop,
    float animSpeed,
    Vec3 scale,
    Vec3 rotation
)
{
    position3D = pos;
    AnimationSpeed = animSpeed;

    if (modelHandle < 0) return;

    // モデルの位置を設定
    MV1SetPosition(modelHandle, DxConv::ToVECTOR(pos));

    // スケールを設定
    MV1SetScale(modelHandle, DxConv::ToVECTOR(scale));

    // 回転を設定（ラジアンで指定）
    MV1SetRotationXYZ(modelHandle, DxConv::ToVECTOR(rotation));

    if (drawAnimModelHandle != modelHandle || drawAnimIndex != animIndex)
    {
        if (drawAnimModelHandle >= 0 && drawAnimAttachIndex >= 0)
        {
            MV1DetachAnim(drawAnimModelHandle, drawAnimAttachIndex);
        }

        drawAnimModelHandle = modelHandle;
        drawAnimIndex = animIndex;
        drawAnimAttachIndex = -1;
        drawAnimElapsed = 0.0f;

        if (animIndex >= 0 && animIndex < MV1GetAnimNum(modelHandle))
        {
            drawAnimAttachIndex = MV1AttachAnim(modelHandle, animIndex);
        }
    }

    if (drawAnimAttachIndex >= 0)
    {
        const float totalTime = MV1GetAttachAnimTotalTime(modelHandle, drawAnimAttachIndex);
        if (totalTime > 0.0f)
        {
            drawAnimElapsed += std::max(0.0f, animSpeed) / 60.0f;
            if (loop)
            {
                drawAnimElapsed = std::fmod(drawAnimElapsed, totalTime);
            }
            else
            {
                drawAnimElapsed = std::min(drawAnimElapsed, totalTime);
            }

            MV1SetAttachAnimTime(modelHandle, drawAnimAttachIndex, drawAnimElapsed);
            currentframe = static_cast<int>(drawAnimElapsed / totalTime * 100.0f);
        }
    }

    // モデルを描画
    MV1DrawModel(modelHandle);
}

// ============================================================================
// 3D描画（MV1モデル版・アニメーションブレンド対応）
// ============================================================================
void AnimationDraw::DrawAnimBlend3D(
    int modelHandle,
    Vec3 pos,
    int currentAnimIndex,
    int nextAnimIndex,
    float blendTime,
    bool loop,
    float animSpeed,
    Vec3 scale,
    Vec3 rotation
)
{
    position3D = pos;
    AnimationSpeed = animSpeed;

    if (modelHandle < 0) return;

    // モデルの位置を設定
    MV1SetPosition(modelHandle, DxConv::ToVECTOR(pos));

    // スケールを設定
    MV1SetScale(modelHandle, DxConv::ToVECTOR(scale));

    // 回転を設定（ラジアンで指定）
    MV1SetRotationXYZ(modelHandle, DxConv::ToVECTOR(rotation));

    // フレーム更新
    int maxFrame = 100;
    UpdateFrameInternal(animSpeed, maxFrame, loop);

    // ブレンド開始時の初期化
    if (attachedAnim0 != currentAnimIndex || attachedAnim1 != nextAnimIndex)
    {
        // 新しいアニメーション組み合わせの場合、初期化
        attachedAnim0 = currentAnimIndex;
        attachedAnim1 = nextAnimIndex;
        blendElapsedTime = 0.0f;
        totalBlendTime = blendTime;

        // スロット0に現在のアニメーションをアタッチ
        MV1AttachAnim(modelHandle, 0, currentAnimIndex, loop ? TRUE : FALSE);
        // スロット1に次のアニメーションをアタッチ
        MV1AttachAnim(modelHandle, 1, nextAnimIndex, loop ? TRUE : FALSE);
    }

    // ブレンド時間を進める
    blendElapsedTime += static_cast<float>(1.0 / 60.0);  // 60FPS想定

    // ブレンド率を計算（0.0 → 1.0）
    float blendRate = 0.0f;
    if (totalBlendTime > 0.0f)
    {
        blendRate = std::min(1.0f, blendElapsedTime / totalBlendTime);
    }

    // スロット0のブレンド率を設定（最初は1.0、最後は0.0）
    MV1SetAttachAnimBlendRate(modelHandle, 0, 1.0f - blendRate);
    // スロット1のブレンド率を設定（最初は0.0、最後は1.0）
    MV1SetAttachAnimBlendRate(modelHandle, 1, blendRate);

    // ブレンドが完了したら、スロット1を新しいメインアニメーションに昇格
    if (blendRate >= 1.0f)
    {
        MV1DetachAnim(modelHandle, 0);
        MV1AttachAnim(modelHandle, 0, nextAnimIndex, loop ? TRUE : FALSE);
        MV1DetachAnim(modelHandle, 1);
        attachedAnim0 = nextAnimIndex;
        attachedAnim1 = -1;
        blendElapsedTime = 0.0f;
    }

    // モデルを描画
    MV1DrawModel(modelHandle);
}
