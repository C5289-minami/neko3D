#pragma once

#include "DxLib.h"

// ===============================
// ---使い方---
// 1. Init() で初期化する。
// 2. BeginScenePass() と EndScenePass() で囲んで、
//    シーンの描画を行う。
// 3. BeginDepthPass() と EndDepthPass() で囲んで、
//    シーンの深度・法線の描画を行う。
// 4. RenderPostProcess() を呼び出して、
//    スクリーン空間アウトライン描画を行う。
// ===============================

// スクリーン空間アウトライン描画を管理するクラス
class ScreenSpaceOutlineRenderer final
{
public:
    ScreenSpaceOutlineRenderer() = default;
    ~ScreenSpaceOutlineRenderer();

    ScreenSpaceOutlineRenderer(const ScreenSpaceOutlineRenderer&) = delete;
    ScreenSpaceOutlineRenderer& operator=(
        const ScreenSpaceOutlineRenderer&
        ) = delete;

    bool Init(int width, int height);
    void Release() noexcept;

    void BeginScenePass() const;
    void EndScenePass() const;

    void BeginDepthPass() const;
    void EndDepthPass() const;

    void RenderPostProcess() const;

    bool IsInitialized() const noexcept;

private:
    int width_{};
    int height_{};

    int depthPixelShader_{ -1 };
    int postProcessPixelShader_{ -1 };

    int sceneBuffer_{ -1 };
    int depthBuffer_{ -1 };
    int settingsConstantBuffer_{ -1 };

    VERTEX2DSHADER fullscreenVertices_[6]{};

    void CreateFullscreenVertices();
};