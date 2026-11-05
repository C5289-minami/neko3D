#pragma once

#include "DxLib.h"
#include "OutlineSettings.h"

// シーンをバッファーに描画し、必要に応じてアウトラインを合成する。
class OutlineRenderer final
{
public:
    OutlineRenderer() = default;
    ~OutlineRenderer();

    OutlineRenderer(const OutlineRenderer&) = delete;
    OutlineRenderer& operator=(const OutlineRenderer&) = delete;

    bool Init(int width, int height);
    void Release() noexcept;

    // シーン描画
    void BeginScenePass() const;
    void EndScenePass() const;

    // 深度・法線の描画
    void BeginDepthPass() const;
    void EndDepthPass() const;

    // オブジェクトIDの描画
    void BeginObjectIdPass() const;
    void EndObjectIdPass() const;

    // アウトラインの合成
    void RenderPostProcess() const;

    // シーン描画と深度・法線・オブジェクトIDの描画をまとめて実行
    template<typename SceneDraw, typename DepthDraw>
    void Render(
        SceneDraw&& sceneDraw,
        DepthDraw&& depthDraw
    ) const;

    void SetOutlineEnabled(bool enabled) noexcept
    {
        outlineEnabled_ = enabled;
    }

    void ClearObjectOutlineSettings() const noexcept;
    void SetObjectOutlineSettings(
        int objectId,
        const OutlineSettings& settings) const noexcept;

private:
    int width_{};
    int height_{};

    // シェーダー
    int depthPixelShader_{ -1 };
    int objectIdPixelShader_{ -1 };
    int postProcessPixelShader_{ -1 };

    // 描画バッファー
    int sceneBuffer_{ -1 };
    int depthBuffer_{ -1 };
    int objectIdBuffer_{ -1 };

    // 設定用定数バッファー
    int settingsConstantBuffer_{ -1 };

    // 全画面描画用頂点
    VERTEX2DSHADER fullscreenVertices_[6]{};

    void CreateFullscreenVertices();

    bool outlineEnabled_ = true;

    void RenderOutlinePass(
        int sourceTexture,
        int destinationScreen
    ) const;
};


// テンプレート関数の定義
template<typename SceneDraw, typename DepthDraw>
inline void OutlineRenderer::Render(
    SceneDraw&& sceneDraw,
    DepthDraw&& depthDraw) const
{
    const int resetSettings = GetUseSetDrawScreenSettingReset();
    SetUseSetDrawScreenSettingReset(FALSE);

    // ① シーン描画
    BeginScenePass();
    sceneDraw();
    EndScenePass();

    if (outlineEnabled_)
    {
        // 毎フレーム深度パスの前にテーブルを現在のグローバル設定で初期化
        ClearObjectOutlineSettings();

        //  深度・法線の描画
        BeginDepthPass();
        depthDraw();
        EndDepthPass();

        //  オブジェクトIDの描画
        BeginObjectIdPass();
        depthDraw();
        EndObjectIdPass();
    }

    //  アウトラインの合成
    RenderPostProcess();

    SetUseSetDrawScreenSettingReset(resetSettings);
}