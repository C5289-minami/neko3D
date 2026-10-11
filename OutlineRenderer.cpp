#include "OutlineRenderer.h"

#include "OutlineSettings.h"

namespace
{
    // ========================================================
    // シェーダー設定
    // ========================================================

    // アウトライン設定を HLSL の定数バッファー b6 に渡す。
    constexpr int OutlineSettingsSlot = 6;

    // 深度・法線情報を出力するシェーダー
    constexpr const wchar_t* DepthShaderPath =
        L"./DevData/ShaderCompiler/Bin/DepthPS.pso";

    // 深度・法線から輪郭を検出し、シーン画像に合成するシェーダー
    constexpr const wchar_t* PostProcessShaderPath =
        L"./DevData/ShaderCompiler/Bin/PostProcessPS.pso";

    // 画像全体にエフェクトを適用するシェーダー
    constexpr const wchar_t* FullScreenPixelShaderPath =
        L"./DevData/ShaderCompiler/Bin/FullScreenPS.pso";
}


// ============================================================
// デストラクタ
// ============================================================

// オブジェクトが破棄されるとき、確保したリソースを解放する。
OutlineRenderer::~OutlineRenderer()
{
    Release();
}


// ============================================================
// Init()
//
// シェーダーや描画用バッファーを作成する。
// ============================================================

bool OutlineRenderer::Init(int width, int height)
{
    // 再初期化にも対応できるよう、以前のリソースを解放する。
    Release();

    // 不正な画面サイズでは初期化しない。
    if (width <= 0 || height <= 0)
    {
        return false;
    }

    width_ = width;
    height_ = height;


    // --------------------------------------------------------
    // ① シェーダーを読み込む
    // --------------------------------------------------------

    depthPixelShader_ = LoadPixelShader(DepthShaderPath);
    postProcessPixelShader_ = LoadPixelShader(PostProcessShaderPath);
    fullScreenPixelShader_ = LoadPixelShader(FullScreenPixelShaderPath);


    // --------------------------------------------------------
    // ② 描画用の画面バッファーを作成する
    // --------------------------------------------------------

    // これから作る画面を、4チャンネル・32ビット浮動小数点形式にする。
    // 深度・法線の情報を精度よく保存するための設定。
    //
    // 注意：
    // この設定を戻すまでの MakeScreen() に適用される。
    // 現在のコードでは depthBuffer_ と sceneBuffer_ の両方が
    // この設定で作成される。
    SetDrawValidFloatTypeGraphCreateFlag(TRUE);
    SetCreateDrawValidGraphChannelNum(4);
    SetCreateGraphChannelBitDepth(32);

    // 深度・法線情報を保存する画面
    depthBuffer_ = MakeScreen(width_, height_, TRUE);

    // 通常のシーン画像を保存する画面
    sceneBuffer_ = MakeScreen(width_, height_, TRUE);


    // これ以降に作る画面へ設定が影響しないよう、元に戻す。
    SetDrawValidFloatTypeGraphCreateFlag(FALSE);
    SetCreateDrawValidGraphChannelNum(0);
    SetCreateGraphChannelBitDepth(0);


    // --------------------------------------------------------
    // ③ アウトライン適用後の画像を一時保存する画面
    // --------------------------------------------------------

    // この画面は上の浮動小数点設定を解除した後に作成している。
    // アウトラインと全画面エフェクトを両方使う際、
    // 全画面エフェクトを適用した途中結果を保存する。
    outlineBuffer_ = MakeScreen(width_, height_, TRUE);


    // --------------------------------------------------------
    // ④ 深度バッファーを有効にする
    // --------------------------------------------------------

    // 3Dモデルの前後関係を正しく判定できるようにする。
    if (sceneBuffer_ >= 0)
    {
        SetUseGraphZBuffer(sceneBuffer_, TRUE);
    }

    if (depthBuffer_ >= 0)
    {
        SetUseGraphZBuffer(depthBuffer_, TRUE);
    }


    // --------------------------------------------------------
    // ⑤ アウトライン設定用の定数バッファーを作成する
    // --------------------------------------------------------

    // OutlineSettings のデータをシェーダーへ渡すための領域。
    settingsConstantBuffer_ =
        CreateShaderConstantBuffer(sizeof(OutlineSettings));


    // 必要なリソースが一つでも作成できなかった場合は失敗。
    if (depthPixelShader_ < 0 ||
        postProcessPixelShader_ < 0 ||
        fullScreenPixelShader_ < 0 ||
        sceneBuffer_ < 0 ||
        depthBuffer_ < 0 ||
        outlineBuffer_ < 0 ||
        settingsConstantBuffer_ < 0)
    {
        Release();
        return false;
    }


    // --------------------------------------------------------
    // ⑥ 定数バッファーに初期設定を格納する
    // --------------------------------------------------------

    auto* settings = static_cast<OutlineSettings*>(
        GetBufferShaderConstantBuffer(settingsConstantBuffer_)
        );

    if (settings == nullptr)
    {
        Release();
        return false;
    }

    // 設定データをコピーする。
    *settings = g_outlineSettings;

    // CPU側で変更した設定をシェーダーへ反映する。
    UpdateShaderConstantBuffer(settingsConstantBuffer_);


    // --------------------------------------------------------
    // ⑦ 全画面描画用の頂点を作成する
    // --------------------------------------------------------

    CreateFullscreenVertices();

    return true;
}


// ============================================================
// Release()
//
// Init() で確保したリソースを解放する。
// ============================================================

void OutlineRenderer::Release() noexcept
{
    // アウトライン設定用の定数バッファー
    if (settingsConstantBuffer_ >= 0)
    {
        DeleteShaderConstantBuffer(settingsConstantBuffer_);
        settingsConstantBuffer_ = -1;
    }

    // アウトライン合成用シェーダー
    if (postProcessPixelShader_ >= 0)
    {
        DeleteShader(postProcessPixelShader_);
        postProcessPixelShader_ = -1;
    }

    // 深度・法線出力用シェーダー
    if (depthPixelShader_ >= 0)
    {
        DeleteShader(depthPixelShader_);
        depthPixelShader_ = -1;
    }

    // シーン画像の画面バッファー
    if (sceneBuffer_ >= 0)
    {
        DeleteGraph(sceneBuffer_);
        sceneBuffer_ = -1;
    }

    // 深度・法線の画面バッファー
    if (depthBuffer_ >= 0)
    {
        DeleteGraph(depthBuffer_);
        depthBuffer_ = -1;
    }

    // 全画面エフェクト用シェーダー
    if (fullScreenPixelShader_ >= 0)
    {
        DeleteShader(fullScreenPixelShader_);
        fullScreenPixelShader_ = -1;
    }

    // アウトライン適用後の一時バッファー
    if (outlineBuffer_ >= 0)
    {
        DeleteGraph(outlineBuffer_);
        outlineBuffer_ = -1;
    }

    // 画面サイズも初期状態へ戻す。
    width_ = 0;
    height_ = 0;
}


// ============================================================
// BeginScenePass()
//
// シーンの描画先を sceneBuffer_ に切り替える。
// ============================================================

void OutlineRenderer::BeginScenePass() const
{
    if (sceneBuffer_ < 0)
    {
        return;
    }

    // 以降の描画先をシーン用バッファーにする。
    SetDrawScreen(sceneBuffer_);

    // シーン用バッファーの深度バッファーを使う。
    SetDrawZBuffer(sceneBuffer_);

    // 3D描画で深度テストを有効にする。
    SetUseZBuffer3D(TRUE);

    // 3D描画で深度値を書き込む。
    SetWriteZBuffer3D(TRUE);

    // DxLibの標準シェーダーでモデルを描画する設定にする。
    // 必要な独自シェーダーは、呼び出し側で設定する。
    MV1SetUseOrigShader(TRUE);

    // 前フレームの色と深度を消去する。
    ClearDrawScreen();
    ClearDrawScreenZBuffer();
}


// ============================================================
// EndScenePass()
//
// シーン描画を終了し、描画先やシェーダーの状態を戻す。
// ============================================================

void OutlineRenderer::EndScenePass() const
{
    // 独自シェーダーを解除する。
    SetUsePixelShader(-1);
    SetUseVertexShader(-1);

    // DxLibの標準モデル描画へ戻す。
    MV1SetUseOrigShader(FALSE);

    // 深度テストと深度書き込みを無効にする。
    SetUseZBuffer3D(FALSE);
    SetWriteZBuffer3D(FALSE);

    // 描画先を通常状態に戻す。
    SetDrawZBuffer(-1);
    SetDrawScreen(DX_SCREEN_BACK);
}


// ============================================================
// BeginDepthPass()
//
// 深度・法線情報を取得するための描画を開始する。
// ============================================================

void OutlineRenderer::BeginDepthPass() const
{
    if (depthBuffer_ < 0)
    {
        return;
    }

    // 深度・法線用の画面へ描画先を切り替える。
    SetDrawScreen(depthBuffer_);

    // この画面の深度バッファーを使う。
    SetDrawZBuffer(depthBuffer_);

    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(TRUE);

    // モデルの標準シェーダーを無効にする。
    // 呼び出し側で使用する頂点シェーダーと、
    // この関数で設定する DepthPS を組み合わせて描画する。
    MV1SetUseOrigShader(TRUE);

    // 前回の深度・法線情報と深度値を消去する。
    ClearDrawScreen();
    ClearDrawScreenZBuffer();

    // ピクセルシェーダーを DepthPS に切り替える。
    SetUsePixelShader(depthPixelShader_);
}


// ============================================================
// EndDepthPass()
//
// 深度・法線の描画を終了する。
// ============================================================

void OutlineRenderer::EndDepthPass() const
{
    // 独自シェーダーを解除する。
    SetUsePixelShader(-1);
    SetUseVertexShader(-1);

    MV1SetUseOrigShader(FALSE);

    // 深度テストと深度書き込みを無効にする。
    SetUseZBuffer3D(FALSE);
    SetWriteZBuffer3D(FALSE);

    // 通常の描画先へ戻す。
    SetDrawZBuffer(-1);
    SetDrawScreen(DX_SCREEN_BACK);
}


// ============================================================
// RenderPostProcess()
//
// 有効になっているエフェクトを適用して、最終画像を表示する。
// ============================================================

void OutlineRenderer::RenderPostProcess() const
{
    // シーン画像がなければ何もできない。
    if (sceneBuffer_ < 0)
    {
        return;
    }

    // アウトラインを使う場合には、深度・法線バッファーと
    // アウトライン用シェーダー、設定バッファーが必要。
    if (outlineEnabled_ &&
        (depthBuffer_ < 0 ||
            postProcessPixelShader_ < 0 ||
            settingsConstantBuffer_ < 0))
    {
        return;
    }

    // 全画面エフェクトを使う場合には、対応するシェーダーが必要。
    if (fullScreenEffectEnabled_ &&
        fullScreenPixelShader_ < 0)
    {
        return;
    }


    // --------------------------------------------------------
    // ケース① アウトラインと全画面エフェクトの両方を使う
    // --------------------------------------------------------

    if (outlineEnabled_ && fullScreenEffectEnabled_)
    {
        // 先にシーン画像へ全画面エフェクトを適用する。
        //
        // 入力：sceneBuffer_
        // 出力：outlineBuffer_
        //
        // 元画像を直接上書きせず、別の画面に結果を保存する。
        if (outlineBuffer_ < 0)
        {
            return;
        }

        RenderFullScreenPass(
            sceneBuffer_,
            outlineBuffer_
        );

        // 次に、全画面エフェクト適用後の画像へ輪郭を合成する。
        //
        // 入力：outlineBuffer_ と depthBuffer_
        // 出力：バックバッファー
        RenderOutlinePass(
            outlineBuffer_,
            DX_SCREEN_BACK
        );
    }

    // --------------------------------------------------------
    // ケース② アウトラインだけを使う
    // --------------------------------------------------------

    else if (outlineEnabled_)
    {
        // シーン画像と深度・法線を使って輪郭を描画する。
        RenderOutlinePass(
            sceneBuffer_,
            DX_SCREEN_BACK
        );
    }

    // --------------------------------------------------------
    // ケース③ 全画面エフェクトだけを使う
    // --------------------------------------------------------

    else if (fullScreenEffectEnabled_)
    {
        // シーン画像に全画面エフェクトを適用する。
        RenderFullScreenPass(
            sceneBuffer_,
            DX_SCREEN_BACK
        );
    }

    // --------------------------------------------------------
    // ケース④ 両方とも使わない
    // --------------------------------------------------------

    else
    {
        // 加工せず、シーン画像をそのまま画面に表示する。
        SetDrawScreen(DX_SCREEN_BACK);
        SetDrawZBuffer(-1);

        DrawGraph(0, 0, sceneBuffer_, FALSE);
    }
}


// ============================================================
// RenderOutlinePass()
//
// sourceTexture の画像に輪郭を合成し、指定画面へ出力する。
// ============================================================

void OutlineRenderer::RenderOutlinePass(
    int sourceTexture,
    int destinationScreen
) const
{
    // 入力画像が無効なら処理しない。
    // 出力先は DX_SCREEN_BACK、または有効な画面ハンドルを指定する。
    if (sourceTexture < 0 ||
        (destinationScreen < 0 &&
            destinationScreen != DX_SCREEN_BACK))
    {
        return;
    }


    // --------------------------------------------------------
    // ① アウトライン設定を定数バッファーへ反映する
    // --------------------------------------------------------

    auto* settings = static_cast<OutlineSettings*>(
        GetBufferShaderConstantBuffer(settingsConstantBuffer_)
        );

    if (settings == nullptr)
    {
        return;
    }

    // 最新のアウトライン設定をコピーする。
    *settings = g_outlineSettings;

    // シェーダーが使う定数バッファーを更新する。
    UpdateShaderConstantBuffer(settingsConstantBuffer_);


    // --------------------------------------------------------
    // ② 描画先と入力テクスチャーを設定する
    // --------------------------------------------------------

    SetDrawScreen(destinationScreen);

    // 全画面の2D描画で、3D用の深度バッファーを使用しない。
    SetDrawZBuffer(-1);

    // アウトライン設定をピクセルシェーダーの b6 に渡す。
    SetShaderConstantBuffer(
        settingsConstantBuffer_,
        DX_SHADERTYPE_PIXEL,
        OutlineSettingsSlot
    );

    // t0：輪郭を付ける元画像
    SetUseTextureToShader(0, sourceTexture);

    // t1：深度・法線情報
    SetUseTextureToShader(1, depthBuffer_);

    // 輪郭検出・合成用シェーダーを使用する。
    SetUsePixelShader(postProcessPixelShader_);


    // --------------------------------------------------------
    // ③ 画面全体にシェーダーを実行する
    // --------------------------------------------------------

    // 全画面を覆う三角形を描画し、
    // ピクセルごとに PostProcessPS を実行する。
    DrawPrimitive2DToShader(
        const_cast<VERTEX2DSHADER*>(fullscreenVertices_),
        6,
        DX_PRIMTYPE_TRIANGLELIST
    );


    // --------------------------------------------------------
    // ④ 使用したシェーダーとテクスチャーを解除する
    // --------------------------------------------------------

    SetUsePixelShader(-1);

    SetUseTextureToShader(0, -1);
    SetUseTextureToShader(1, -1);

    // 次の描画に備えてバックバッファーへ戻す。
    SetDrawScreen(DX_SCREEN_BACK);
}


// ============================================================
// IsInitialized()
//
// 初期化に必要なリソースが揃っているかを調べる。
// ============================================================

bool OutlineRenderer::IsInitialized() const noexcept
{
    return depthPixelShader_ >= 0 &&
        postProcessPixelShader_ >= 0 &&
        fullScreenPixelShader_ >= 0 &&
        sceneBuffer_ >= 0 &&
        depthBuffer_ >= 0 &&
        outlineBuffer_ >= 0 &&
        settingsConstantBuffer_ >= 0;
}


// ============================================================
// RenderFullScreenShader()
//
// sceneBuffer_ に保存された画像に全画面エフェクトを適用し、
// バックバッファーへ出力する。
// ============================================================

void OutlineRenderer::RenderFullScreenShader() const
{
    // シーン画像に全画面エフェクトを適用し、
    // バックバッファーへ描画する
    RenderFullScreenPass(
        sceneBuffer_,
        DX_SCREEN_BACK
    );
}


// ============================================================
// RenderFullScreenPass()
//
// sourceTexture に全画面エフェクトを適用して、
// destinationScreen へ出力する。
// ============================================================

void OutlineRenderer::RenderFullScreenPass(
    int sourceTexture,
    int destinationScreen
) const
{
    if (sourceTexture < 0 ||
        fullScreenPixelShader_ < 0)
    {
        return;
    }

    // 出力先を指定する。
    SetDrawScreen(destinationScreen);

    // 3D用の深度バッファーは使わない。
    SetDrawZBuffer(-1);

    // 入力画像を t0 に設定する。
    SetUseTextureToShader(0, sourceTexture);

    // 全画面エフェクト用シェーダーを設定する。
    SetUsePixelShader(fullScreenPixelShader_);

    // 出力先全体にエフェクトを適用する。
    DrawPrimitive2DToShader(
        const_cast<VERTEX2DSHADER*>(fullscreenVertices_),
        6,
        DX_PRIMTYPE_TRIANGLELIST
    );

    // シェーダーと入力テクスチャーを解除する。
    SetUsePixelShader(-1);

    SetUseTextureToShader(0, -1);
    SetUseTextureToShader(1, -1);

    // 通常の描画先へ戻す。
    SetDrawScreen(DX_SCREEN_BACK);
}


// ============================================================
// CreateFullscreenVertices()
//
// 画面全体を覆う2枚の三角形を作る。
// 全画面シェーダーを画面の隅々まで実行するために使用する。
// ============================================================

void OutlineRenderer::CreateFullscreenVertices()
{
    const float width = static_cast<float>(width_);
    const float height = static_cast<float>(height_);


    // 画面左上を (0, 0)、右下を (width, height) とする。
    // 6頂点で四角形を2枚の三角形として構成する。
    const VECTOR positions[6] =
    {
        VGet(0.0f, 0.0f, 0.0f),
        VGet(width, 0.0f, 0.0f),
        VGet(0.0f, height, 0.0f),

        VGet(width, height, 0.0f),
        VGet(0.0f, height, 0.0f),
        VGet(width, 0.0f, 0.0f)
    };


    // 画面左上が (0, 0)、右下が (1, 1) になるUV。
    // 画面画像をテクスチャーとして全面に貼る。
    const float uvs[6][2] =
    {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 0.0f, 1.0f },

        { 1.0f, 1.0f },
        { 0.0f, 1.0f },
        { 1.0f, 0.0f }
    };


    // 6頂点分の位置・色・UVを初期化する。
    for (int i = 0; i < 6; ++i)
    {
        // 頂点の画面座標
        fullscreenVertices_[i].pos = positions[i];

        // 2D描画用の補正値
        fullscreenVertices_[i].rhw = 1.0f;

        // エフェクト用のテクスチャーを白く描画する。
        // 入力画像そのものの色をシェーダーで扱うため、
        // 頂点色によって色を変えない設定。
        fullscreenVertices_[i].dif =
            GetColorU8(255, 255, 255, 255);

        // スペキュラ色は使わない。
        fullscreenVertices_[i].spc =
            GetColorU8(0, 0, 0, 0);

        // テクスチャー座標
        fullscreenVertices_[i].u = uvs[i][0];
        fullscreenVertices_[i].v = uvs[i][1];

        // サブテクスチャー座標も同じ値にする。
        fullscreenVertices_[i].su = uvs[i][0];
        fullscreenVertices_[i].sv = uvs[i][1];
    }
}
