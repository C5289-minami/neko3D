#pragma once

#include "DxLib.h"

// ============================================================
// OutlineRenderer
//
// アウトライン描画と全画面エフェクトを管理するクラス。
//
// 主な仕事
// ・シーン描画用のバッファーを管理する
// ・深度、法線を取得するための描画を管理する
// ・アウトラインを描画する
// ・全画面エフェクトを適用する
//
// 基本的な使い方
// 1. Init() で初期化する。
// 2. シーンを sceneBuffer_ に描画する。
// 3. アウトラインが必要なら深度・法線を描画する。
// 4. RenderPostProcess() で画像を加工して画面に表示する。
// 5. 不要になったら Release() でリソースを解放する。
// ============================================================


// ------------------------------------------------------------
// このクラスの Render() を使用する場合の例
//
// outlineRenderer_.SetOutlineEnabled(true);
// outlineRenderer_.SetFullScreenEffectEnabled(false);
//
// outlineRenderer_.Render(
//     [&]() { シーンの描画処理; },
//     [&]() { 深度・法線の描画処理; }
// );
//
// アウトラインのみ：
// SetOutlineEnabled(true);
// SetFullScreenEffectEnabled(false);
//
// 全画面エフェクトのみ：
// SetOutlineEnabled(false);
// SetFullScreenEffectEnabled(true);
//
// 両方無効：
// SetOutlineEnabled(false);
// SetFullScreenEffectEnabled(false);
//
// ※全画面エフェクトのみの場合、Render() の2番目の
//   コールバックは実行されない。
// ------------------------------------------------------------


class OutlineRenderer final
{
public:
    OutlineRenderer() = default;
    ~OutlineRenderer();

    // DxLibのシェーダーや画面バッファーはハンドルで管理する。
    // コピーすると、同じハンドルを二重に解放する危険があるため禁止する。
    OutlineRenderer(const OutlineRenderer&) = delete;
    OutlineRenderer& operator=(const OutlineRenderer&) = delete;


    // ========================================================
    // 初期化・解放
    // ========================================================

    // シェーダー、画面バッファー、定数バッファーを作成する。
    // width、height は描画する画面のサイズ。
    bool Init(int width, int height);

    // 作成したシェーダーやバッファーを解放する。
    void Release() noexcept;


    // ========================================================
    // シーン描画
    // ========================================================

    // これ以降の描画先をシーン用バッファーへ変更する。
    void BeginScenePass() const;

    // シーン描画を終了し、描画先などの状態を戻す。
    void EndScenePass() const;


    // ========================================================
    // 深度・法線描画
    // ========================================================

    // 描画先を深度・法線用バッファーへ変更し、
    // DepthPS を設定する。
    //
    // この関数自身はモデルを描画しない。
    // 呼び出し側で頂点シェーダーを設定し、モデルを描画する。
    void BeginDepthPass() const;

    // 深度・法線の描画を終了する。
    void EndDepthPass() const;


    // ========================================================
    // ポストプロセス
    // ========================================================

    // 設定に応じてアウトラインや全画面エフェクトを実行する。
    // 最終結果はバックバッファーへ描画される。
    void RenderPostProcess() const;

    // 初期化に成功しているか確認する。
    bool IsInitialized() const noexcept;


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


    // ========================================================
    // 全画面エフェクト専用
    // ========================================================

    // sceneBuffer_ の画像に全画面シェーダーを適用する。
    // この関数は有効フラグを確認せず、直接エフェクトを実行する。
    void RenderFullScreenShader() const;

    // シーンの描画から全画面エフェクトまでをまとめて実行する。
    // この関数も全画面シェーダーを直接実行する。
    template<typename SceneDraw>
    void RenderFullScreenEffect(SceneDraw&& sceneDraw) const;


    // ========================================================
    // エフェクトの有効・無効
    // ========================================================

    // true：アウトラインを適用する
    // false：アウトラインを適用しない
    void SetOutlineEnabled(bool enabled) noexcept
    {
        outlineEnabled_ = enabled;
    }

    // true：全画面エフェクトを適用する
    // false：全画面エフェクトを適用しない
    //
    // このフラグは RenderPostProcess() 内で使用する。
    void SetFullScreenEffectEnabled(bool enabled) noexcept
    {
        fullScreenEffectEnabled_ = enabled;
    }


private:

    // ========================================================
    // 画面サイズ
    // ========================================================

    int width_{};
    int height_{};


    // ========================================================
    // シェーダーハンドル
    // ========================================================

    // 深度・法線情報を書き込むピクセルシェーダー
    int depthPixelShader_{ -1 };

    // 深度・法線を使って輪郭を検出・合成するシェーダー
    int postProcessPixelShader_{ -1 };

    // 画像全体にエフェクトを適用するシェーダー
    int fullScreenPixelShader_{ -1 };


    // ========================================================
    // 描画用の画面バッファー
    // ========================================================

    // 通常のシーン描画結果を保存する。
    int sceneBuffer_{ -1 };

    // 深度・法線情報を保存する。
    int depthBuffer_{ -1 };

    // PostProcessPS に渡すアウトライン設定用定数バッファー
    int settingsConstantBuffer_{ -1 };

    // アウトライン適用後の画像を一時保存する。
    // アウトラインと全画面エフェクトの両方を使うときに使用する。
    int outlineBuffer_{ -1 };


    // ========================================================
    // 全画面描画用の頂点
    // ========================================================

    // 画面全体を覆う2枚の三角形を構成する6頂点。
    // 全画面エフェクトやアウトラインの合成に使う。
    VERTEX2DSHADER fullscreenVertices_[6]{};

    // 全画面描画用の頂点を初期化する。
    void CreateFullscreenVertices();


    // ========================================================
    // エフェクト設定
    // ========================================================

    // アウトラインと全画面エフェクトの有効状態
    bool outlineEnabled_ = true;
    bool fullScreenEffectEnabled_ = false;


    // ========================================================
    // 内部処理
    // ========================================================

    // sourceTexture の画像にアウトラインを適用し、
    // destinationScreen へ描画する。
    void RenderOutlinePass(
        int sourceTexture,
        int destinationScreen
    ) const;

    // sourceTexture の画像に全画面エフェクトを適用し、
    // destinationScreen へ描画する。
    void RenderFullScreenPass(
        int sourceTexture,
        int destinationScreen
    ) const;
};


// ============================================================
// Render()
//
// 1. シーンを描画する。
// 2. アウトラインが有効なら、深度・法線を描画する。
// 3. 設定に応じてポストプロセスを実行する。
// ============================================================

template<typename SceneDraw, typename DepthDraw>
void OutlineRenderer::Render(
    SceneDraw&& sceneDraw,
    DepthDraw&& depthDraw
) const
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


// ============================================================
// RenderFullScreenEffect()
//
// 1. シーンをバッファーに描画する。
// 2. 全画面シェーダーを適用する。
//
// この関数はアウトラインを実行しない。
// また、fullScreenEffectEnabled_ の値に関係なく、
// 全画面シェーダーを直接実行する。
// ============================================================

template<typename SceneDraw>
void OutlineRenderer::RenderFullScreenEffect(
    SceneDraw&& sceneDraw
) const
{
    BeginScenePass();
    sceneDraw();
    EndScenePass();

    RenderFullScreenShader();
}
