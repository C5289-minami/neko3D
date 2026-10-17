#include "ModelToonRenderer.h"

#include "DrawableObject.h"
#include "DxLib.h"
#include "ToonSettings.h"

namespace
{
    // ModelToonPS.hlsl の ToonSettings は b5 を使用している
    constexpr int ToonSettingsSlot = 5;

    // シェーダーの読み込み先
    constexpr const wchar_t* PixelShaderPath =
        L"./Data/Shader/Bin/ModelToonPS.pso";

    constexpr const wchar_t* VertexShaderPath =
        L"./Data/Shader/Bin/ModelVS.vso";

    constexpr const wchar_t* BoneVertexShaderPath =
        L"./Data/Shader/Bin/ModelVS_Bone.vso";
}


// オブジェクト破棄時にGPUリソースを解放する
ModelToonRenderer::~ModelToonRenderer()
{
    Release();
}


// ============================================================
// 初期化
// ============================================================

bool ModelToonRenderer::Init()
{
    // 再初期化にも対応できるよう、既存リソースを解放する
    Release();

    // シェーダーを読み込む
    pixelShader_ = LoadPixelShader(PixelShaderPath);
    vertexShader_ = LoadVertexShader(VertexShaderPath);
    vertexShaderBone_ = LoadVertexShader(BoneVertexShaderPath);

    // ToonSettings をGPUへ渡す定数バッファーを作成する
    toonConstantBuffer_ =
        CreateShaderConstantBuffer(sizeof(ToonSettings));

    // 必要なリソースが作成できなければ初期化を中断する
    if (pixelShader_ < 0 ||
        vertexShader_ < 0 ||
        vertexShaderBone_ < 0 ||
        toonConstantBuffer_ < 0)
    {
        Release();
        return false;
    }
	return true;
}


// ============================================================
// リソース解放
// ============================================================

void ModelToonRenderer::Release() noexcept
{
    if (toonConstantBuffer_ >= 0)
    {
        DeleteShaderConstantBuffer(toonConstantBuffer_);
        toonConstantBuffer_ = -1;
    }

    if (pixelShader_ >= 0)
    {
        DeleteShader(pixelShader_);
        pixelShader_ = -1;
    }

    if (vertexShader_ >= 0)
    {
        DeleteShader(vertexShader_);
        vertexShader_ = -1;
    }

    if (vertexShaderBone_ >= 0)
    {
        DeleteShader(vertexShaderBone_);
        vertexShaderBone_ = -1;
    }
}


// ============================================================
// トゥーン描画開始
// ============================================================

void ModelToonRenderer::BeginPass() const
{
    if (pixelShader_ < 0 || toonConstantBuffer_ < 0)
    {
        return;
    }

    // 定数バッファーのメモリを取得する
    auto* settings = static_cast<ToonSettings*>(
        GetBufferShaderConstantBuffer(toonConstantBuffer_)
        );

    if (settings == nullptr)
    {
        return;
    }

    // 最新のトゥーン設定を反映する
    *settings = g_toonSettings;

    // 現在のライト方向も反映する
    const VECTOR lightDirection = GetLightDirection();

    settings->LightDirection[0] = lightDirection.x;
    settings->LightDirection[1] = lightDirection.y;
    settings->LightDirection[2] = lightDirection.z;
    settings->LightDirection[3] = 0.0f;

    // GPU側の定数バッファーを更新する
    UpdateShaderConstantBuffer(toonConstantBuffer_);

    // HLSL の register(b5) に対応する場所へ設定値を渡す
    SetShaderConstantBuffer(
        toonConstantBuffer_,
        DX_SHADERTYPE_PIXEL,
        ToonSettingsSlot
    );

    // トゥーンの色を計算するピクセルシェーダーを設定する
    SetUsePixelShader(pixelShader_);
}


// ============================================================
// トゥーン描画終了
// ============================================================

void ModelToonRenderer::EndPass() const
{
    // このクラスで設定したシェーダーを解除する
    SetUsePixelShader(-1);
    SetUseVertexShader(-1);
}


// ============================================================
// モデル描画
// ============================================================

void ModelToonRenderer::Draw(
    const ModelObject& model,
    ModelToonType type
) const
{
    // モデルに合った頂点シェーダーを選択する
    SetVertexShader(type);

    // 実際のモデル描画
    model.Draw();
}


// ============================================================
// 頂点シェーダー切り替え
// ============================================================

void ModelToonRenderer::SetVertexShader(
    ModelToonType type
) const
{
    switch (type)
    {
    case ModelToonType::Model:
        SetUseVertexShader(vertexShader_);
        break;

    case ModelToonType::Bone:
        SetUseVertexShader(vertexShaderBone_);
        break;
    }
}