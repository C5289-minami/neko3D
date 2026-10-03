#pragma once
#include "DxPlus/DxPlus.h"
#include "Vector3.h"

class AnimationDraw {
public:
    AnimationDraw() = default;
    virtual ~AnimationDraw() = default;

    // ========== 2D描画（スプライト版） ==========
    virtual void DrawAnim(
        DxPlus::Vec2 pos, 
        const std::vector<int>& sprite, 
        bool loop, 
        float animSpeed,
        DxPlus::Vec2 center = { 0.0f, 0.0f },
        DxPlus::Vec2 scale = { 1.0f, 1.0f },
        float angle = 0.0f,
        int color = GetColor(255, 255, 255)
    );

    virtual void DrawAnim(
        DxPlus::Vec2 pos,
        const std::vector<int>& sprite,
        bool loop,
        float animSpeed,
        DxPlus::Vec2 center,
        float scale,
        float angle = 0.0f,
        int color = GetColor(255, 255, 255)
    );

    // ========== 3D描画（MV1モデル版） ==========
    /// <summary>
    /// 3D MV1モデルのアニメーション描画
    /// </summary>
    /// <param name="modelHandle">MV1モデルハンドル</param>
    /// <param name="pos">3D世界座標</param>
    /// <param name="animIndex">アニメーションインデックス</param>
    /// <param name="loop">ループ再生の有無</param>
    /// <param name="animSpeed">アニメーション速度</param>
    /// <param name="scale">スケール（デフォルト {1,1,1}）</param>
    /// <param name="rotation">回転角度（ラジアン、デフォルト {0,0,0}）</param>
    virtual void DrawAnim3D(
        int modelHandle,
        Vec3 pos,
        int animIndex,
        bool loop,
        float animSpeed,
        Vec3 scale = { 1.0f, 1.0f, 1.0f },
        Vec3 rotation = { 0.0f, 0.0f, 0.0f }
    );

    /// <summary>
    /// 2つのアニメーション間をブレンド（クロスフェード）して再生
    /// 現在のアニメーションから次のアニメーションへ滑らかに遷移します
    /// </summary>
    /// <param name="modelHandle">MV1モデルハンドル</param>
    /// <param name="pos">3D世界座標</param>
    /// <param name="currentAnimIndex">現在のアニメーションインデックス</param>
    /// <param name="nextAnimIndex">次のアニメーションインデックス</param>
    /// <param name="blendTime">ブレンド時間（秒）</param>
    /// <param name="loop">ループ再生の有無</param>
    /// <param name="animSpeed">アニメーション速度</param>
    /// <param name="scale">スケール（デフォルト {1,1,1}）</param>
    /// <param name="rotation">回転角度（ラジアン、デフォルト {0,0,0}）</param>
    virtual void DrawAnimBlend3D(
        int modelHandle,
        Vec3 pos,
        int currentAnimIndex,
        int nextAnimIndex,
        float blendTime,
        bool loop,
        float animSpeed,
        Vec3 scale = { 1.0f, 1.0f, 1.0f },
        Vec3 rotation = { 0.0f, 0.0f, 0.0f }
    );

    // ========== 取得・操作 ==========
    int GetCurrentFrame() const { return currentframe; }
    void Reset() { 
        if (drawAnimModelHandle >= 0 && drawAnimAttachIndex >= 0)
        {
            MV1DetachAnim(drawAnimModelHandle, drawAnimAttachIndex);
        }
        currentframe = 0;
        currentInterval = 0.0f;
        spriteNum = -1;
        frameInterval = 0.0f;
        drawAnimModelHandle = -1;
        drawAnimIndex = -1;
        drawAnimAttachIndex = -1;
        drawAnimElapsed = 0.0f;
    }

protected:
    int spriteNum = -1;
    float currentInterval = 0.0f;
    int currentframe = 0;
    float frameInterval = 0.0f;
    DxPlus::Vec2 position = { 0.0f, 0.0f };
    Vec3 position3D = { 0.0f, 0.0f, 0.0f };
    float AnimationSpeed = 1.0f;

    // アニメーションブレンド用の状態管理
    int attachedAnim0 = -1;  // スロット0のアニメーションインデックス
    int attachedAnim1 = -1;  // スロット1のアニメーションインデックス
    float blendElapsedTime = 0.0f;  // ブレンド経過時間
    float totalBlendTime = 0.0f;    // 総ブレンド時間
    int drawAnimModelHandle = -1;
    int drawAnimIndex = -1;
    int drawAnimAttachIndex = -1;
    float drawAnimElapsed = 0.0f;

    // 内部ヘルパー関数
    void UpdateFrameInternal(float speed, int spriteCount, bool loop);
};
