#include "ModelToonRenderer.h"

#include "DrawableObject.h"
#include "DxLib.h"
#include "ToonSettings.h"
#include "ResourceManager.h"

namespace
{
    constexpr int ToonSettingsSlot = 5;

    constexpr const wchar_t* PixelShaderPath =
        L"./Data/Shader/Bin/ModelToonPS.pso";

    constexpr const wchar_t* VertexShaderPath =
        L"./Data/Shader/Bin/ModelVS.vso";

    constexpr const wchar_t* BoneVertexShaderPath =
        L"./Data/Shader/Bin/ModelVS_Bone.vso";

    constexpr const wchar_t* BoneVertexShader8FramePath =
        L"./Data/Shader/Bin/ModelVS_Bone8Frame.vso";
}


// ============================================================
// 生成・破棄
// ============================================================

ModelToonRenderer::~ModelToonRenderer()
{
    Release();
}


// ============================================================
// 初期化
// ============================================================

bool ModelToonRenderer::Init()
{
    Release();

    pixelShader_ = LoadPixelShader(PixelShaderPath);
    vertexShader_ = LoadVertexShader(VertexShaderPath);
    vertexShaderBone_ = LoadVertexShader(BoneVertexShaderPath);
    vertexShaderBone8Frame_ =
        LoadVertexShader(BoneVertexShader8FramePath);

    toonConstantBuffer_ =
        CreateShaderConstantBuffer(sizeof(ToonSettings));

    if (pixelShader_ < 0 ||
        vertexShader_ < 0 ||
        vertexShaderBone_ < 0 ||
        vertexShaderBone8Frame_ < 0 ||
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

    if (vertexShaderBone8Frame_ >= 0)
    {
        DeleteShader(vertexShaderBone8Frame_);
        vertexShaderBone8Frame_ = -1;
    }
}


// ============================================================
// トゥーン描画開始・終了
// ============================================================

void ModelToonRenderer::BeginPass() const
{
    if (pixelShader_ < 0 || toonConstantBuffer_ < 0)
    {
        return;
    }

    auto* settings = static_cast<ToonSettings*>(
        GetBufferShaderConstantBuffer(toonConstantBuffer_)
        );

    if (settings == nullptr)
    {
        return;
    }

    *settings = g_toonSettings;

    const VECTOR lightDirection = GetLightDirection();

    settings->LightDirection[0] = lightDirection.x;
    settings->LightDirection[1] = lightDirection.y;
    settings->LightDirection[2] = lightDirection.z;
    settings->LightDirection[3] = 0.0f;

    UpdateShaderConstantBuffer(toonConstantBuffer_);

    SetShaderConstantBuffer(
        toonConstantBuffer_,
        DX_SHADERTYPE_PIXEL,
        ToonSettingsSlot
    );

    SetUsePixelShader(pixelShader_);
}

void ModelToonRenderer::EndPass() const
{
    SetUsePixelShader(-1);
    SetUseVertexShader(-1);
}


// ============================================================
// モデル描画
// ============================================================

void ModelToonRenderer::Draw(
    const ModelObject& model,
    ModelType type
) const
{
    if (pixelShader_ < 0)
    {
        return;
    }

    // 頂点シェーダーはモデル全体の種類ではなく、
    // トライアングルリストごとの頂点タイプで選択する。
    (void)type;

    SetUsePixelShader(pixelShader_);

    DrawTriangleLists(model);
}

void ModelToonRenderer::DrawDepth(
    const ModelObject& model
) const
{
    DrawTriangleLists(model);
}


// ============================================================
// 頂点シェーダー切り替え
// ============================================================

void ModelToonRenderer::SetVertexShader(ModelType type) const
{
    switch (type)
    {
    case ModelType::Model:
        SetUseVertexShader(vertexShader_);
        break;

    case ModelType::Bone:
        SetUseVertexShader(vertexShaderBone_);
        break;
    }
}


// ============================================================
// トライアングルリスト描画
// ============================================================

void ModelToonRenderer::DrawTriangleLists(
    const ModelObject& model
) const
{
    const int handle = RM().GetModel(model.GetModelKey());

    if (handle < 0)
    {
        return;
    }

    model.ApplyTransform();
    model.ApplyCollisionColor();

    const int listCount = MV1GetTriangleListNum(handle);

    for (int i = 0; i < listCount; ++i)
    {
        const int vertexType =
            MV1GetTriangleListVertexType(handle, i);

        int shader = -1;

        switch (vertexType)
        {
        case DX_MV1_VERTEX_TYPE_1FRAME:
        case DX_MV1_VERTEX_TYPE_NMAP_1FRAME:
            shader = vertexShader_;
            break;

        case DX_MV1_VERTEX_TYPE_4FRAME:
        case DX_MV1_VERTEX_TYPE_NMAP_4FRAME:
            shader = vertexShaderBone_;
            break;

        case DX_MV1_VERTEX_TYPE_8FRAME:
        case DX_MV1_VERTEX_TYPE_NMAP_8FRAME:
            shader = vertexShaderBone8Frame_;
            break;

        default:
            // 対応していない頂点タイプはスキップする。
            continue;
        }

        if (shader < 0)
        {
            continue;
        }

        SetUseVertexShader(shader);

        MV1DrawTriangleList(handle, i);
    }
}