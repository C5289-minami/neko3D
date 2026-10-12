#pragma once

#include "DxLib.h"

// シーンをバッファーに描画し、必要に応じてアウトラインを合成する。


class OutlineRenderer final
{
public:
    OutlineRenderer() = default;
    ~OutlineRenderer();

   
    // コピー禁止
    OutlineRenderer(const OutlineRenderer&) = delete;
    OutlineRenderer& operator=(const OutlineRenderer&) = delete;

    bool Init(int width, int height);
    void Release() noexcept;


    // これ以降の描画先をシーン用バッファーへ変更する。
    void BeginScenePass() const;
    // シーン描画を終了し、描画先などの状態を戻す。
    void EndScenePass() const;

    // 呼び出し側で頂点シェーダーを設定し、モデルを描画する。
    void BeginDepthPass() const;
    // 深度・法線の描画を終了する。
    void EndDepthPass() const;

    // アウトラインを合成し、バックバッファーへ出力する。
    void RenderPostProcess() const;


    // ========================================================
    // 描画の一連の流れをまとめる関数
    // ========================================================

    // シーン描画と深度・法線描画を、呼び出し側から渡す。
    //
    // SceneDraw：通常のシーンを描画する処理
    // DepthDraw：深度・法線を描画する処理
    //
    // 実際のモデル描画はシーン側で行う。
    template<typename SceneDraw, typename DepthDraw>
    void Render(
        SceneDraw&& sceneDraw,
        DepthDraw&& depthDraw
    ) const;

	// アウトラインの有効・無効を設定する。
    void SetOutlineEnabled(bool enabled) noexcept
    {
        outlineEnabled_ = enabled;
    }


private:
    // 画面サイズ
    int width_{};
    int height_{};

    // Shader

    // 深度・法線情報を書き込むピクセルシェーダー
    int depthPixelShader_{ -1 };
    // 深度・法線を使って輪郭を検出・合成するシェーダー
    int postProcessPixelShader_{ -1 };

    // バッファー
    // 通常のシーン描画結果を保存する。
    int sceneBuffer_{ -1 };
    // 深度・法線情報を保存する。
    int depthBuffer_{ -1 };
    // PostProcessPS に渡すアウトライン設定用定数バッファー
    int settingsConstantBuffer_{ -1 };


    // ========================================================
    // 全画面描画用の頂点
    // ========================================================
    
    // アウトライン合成用の全画面描画頂点
    VERTEX2DSHADER fullscreenVertices_[6]{};

    // 全画面描画用の頂点を初期化する。
    void CreateFullscreenVertices();

	// ========================================================
	// 設定
	// ========================================================

    // アウトラインの有効状態
    bool outlineEnabled_ = true;


    // ========================================================
    // 内部処理
    // ========================================================

    // sourceTexture の画像にアウトラインを適用し、
    // destinationScreen へ描画する。
    void RenderOutlinePass(
        int sourceTexture,
        int destinationScreen
    ) const;

};


// シーン描画と、必要な場合の深度・法線描画を実行する。
template<typename SceneDraw, typename DepthDraw>
inline void OutlineRenderer::Render(SceneDraw&& sceneDraw, DepthDraw&& depthDraw) const
{
    // ① シーンの色をバッファーに保存する
    BeginScenePass();
    sceneDraw();
    EndScenePass();

    // ② アウトラインが必要なときだけ深度・法線を保存する
    if (outlineEnabled_)
    {
        BeginDepthPass();
        depthDraw();
        EndDepthPass();
    }

    // ③ フラグの組み合わせに応じて画像を加工する
    RenderPostProcess();
}
