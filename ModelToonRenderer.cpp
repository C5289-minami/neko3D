#include "ModelToonRenderer.h"

#include "DrawableObject.h"
#include "DxLib.h"
#include "ToonSettings.h"
#include "OutlineSettings.h"
#include "ResourceManager.h"
#include <algorithm>


namespace
{
    constexpr int ToonSettingsSlot = 5;

    constexpr const wchar_t* PixelShaderPath =
        L"./Data/Shader/Bin/ModelToonPS.pso";

    constexpr const wchar_t* VertexShaderPath =
        L"./Data/Shader/Bin/ModelVS.vso";

    constexpr const wchar_t* BoneVertexShaderPath =
        L"./Data/Shader/Bin/ModelVS_Bone.vso";

    constexpr const wchar_t* BoneVertexShaderNMapPath =
        L"./Data/Shader/Bin/ModelVS_BoneNMap.vso";

    constexpr const wchar_t* BoneVertexShader8FramePath =
        L"./Data/Shader/Bin/ModelVS_Bone8Frame.vso";
    constexpr const wchar_t* BoneVertexShader8FrameNMapPath =
        L"./Data/Shader/Bin/ModelVS_Bone8FrameNMap.vso";
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
    vertexShaderBoneNMap_ = LoadVertexShader(BoneVertexShaderNMapPath);
    vertexShaderBone8Frame_ =
        LoadVertexShader(BoneVertexShader8FramePath);
    vertexShaderBone8FrameNMap_ =
        LoadVertexShader(BoneVertexShader8FrameNMapPath);


    toonConstantBuffer_ =
        CreateShaderConstantBuffer(sizeof(ToonSettings));
    depthObjectIdConstantBuffer_ =
        CreateShaderConstantBuffer(sizeof(DepthObjectIdConstants));

    if (pixelShader_ < 0 ||
        vertexShader_ < 0 ||
        vertexShaderBone_ < 0 ||
        vertexShaderBoneNMap_ < 0 ||
        vertexShaderBone8Frame_ < 0 ||
        vertexShaderBone8FrameNMap_ < 0 ||
        depthObjectIdConstantBuffer_ < 0 ||
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
    triangleListsCache_.clear();

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

    if (vertexShaderBoneNMap_ >= 0)
    {
        DeleteShader(vertexShaderBoneNMap_);
        vertexShaderBoneNMap_ = -1;
    }

    if (vertexShaderBone8Frame_ >= 0)
    {
        DeleteShader(vertexShaderBone8Frame_);
        vertexShaderBone8Frame_ = -1;
    }

    if (vertexShaderBone8FrameNMap_ >= 0)
    {
        DeleteShader(vertexShaderBone8FrameNMap_);
        vertexShaderBone8FrameNMap_ = -1;
    }

    if (depthObjectIdConstantBuffer_ >= 0)
    {
        DeleteShaderConstantBuffer(depthObjectIdConstantBuffer_);
        depthObjectIdConstantBuffer_ = -1;
    }
    depthObjectIds_.clear();
    nextDepthObjectId_ = 1;
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
    const ModelObject& model
) const
{
    if (pixelShader_ < 0)
    {
        return;
    }

    SetUsePixelShader(pixelShader_);

    DrawTriangleLists(model, false);
}

void ModelToonRenderer::DrawDepth(const ModelObject& model) const
{
    // 完全に透明（0.0f以下）のときだけスキップする
    if (model.GetAlpha() <= 0.0f)
    {
        return;
    }

    auto it = depthObjectIds_.find(&model);

    if (it == depthObjectIds_.end())
    {
        const int objectId = (std::min)(nextDepthObjectId_++, MAX_OUTLINE_OBJECTS);
        it = depthObjectIds_.emplace(&model, objectId).first;
    }

    DrawDepth(model, it->second);
}

void ModelToonRenderer::DrawDepth(
    const ModelObject& model,
    int objectId) const
{
    // 完全に透明（0.0f以下）のときだけスキップする
    if (model.GetAlpha() <= 0.0f)
    {
        return;
    }
    if (depthObjectIdConstantBuffer_ < 0)
    {
        return;
    }

    // --------------------------------------------------------
    // モデル固有のアウトライン設定と SetAlpha の値をテーブルへ反映
    // --------------------------------------------------------
    if (objectId >= 1 && objectId <= MAX_OUTLINE_OBJECTS)
    {
        OutlineSettings modelOutline = model.GetOutlineSettings();

        const float modelAlpha = std::clamp(model.GetAlpha(), 0.0f, 1.0f);
        modelOutline.outerOpacity *= modelAlpha;
        modelOutline.innerOpacity *= modelAlpha;
        modelOutline.outlineColor[3] *= modelAlpha;

        g_outlineSettingsTable.table[objectId] = modelOutline;
    }

    auto* settings = static_cast<DepthObjectIdConstants*>(
        GetBufferShaderConstantBuffer(depthObjectIdConstantBuffer_));

    if (settings == nullptr)
    {
        return;
    }

    settings->objectId = static_cast<float>(objectId);
    settings->padding[0] = 0.0f;
    settings->padding[1] = 0.0f;
    settings->padding[2] = 0.0f;

    UpdateShaderConstantBuffer(depthObjectIdConstantBuffer_);

    SetShaderConstantBuffer(
        depthObjectIdConstantBuffer_,
        DX_SHADERTYPE_PIXEL,
        7
    );

    // 深度・オブジェクトIDパスでは半透明モデルでもZバッファへ書き込む
    DrawTriangleLists(model, true);
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
    const ModelObject& model,
    bool forceWriteZ
) const
{
    const int handle = RM().GetModel(model.GetModelKey());
    if (handle < 0) return;

    // モデルごとの不透明度をシェーダーに渡す
    auto* settings = static_cast<ToonSettings*>(
        GetBufferShaderConstantBuffer(toonConstantBuffer_));

    if (settings == nullptr) return;


    settings->modelColor[3] = model.GetAlpha();

    UpdateShaderConstantBuffer(toonConstantBuffer_);

    // Zバッファを使用する
    MV1SetUseZBuffer(handle, TRUE);

    // 深度パス（forceWriteZ == true）または不透明モデルならZを書き込む
    MV1SetWriteZBuffer(handle, (forceWriteZ || model.GetAlpha() >= 1.0f) ? TRUE : FALSE);

    // モデルの不透明度を設定
    MV1SetOpacityRate(handle, model.GetAlpha());

    model.ApplyDrawTransform();
    model.ApplyCollisionColor();

    // モデル情報は初回だけ取得する
    auto& cache = triangleListsCache_[handle];

    if (!cache.initialized)
    {
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
                shader = vertexShaderBone_;
                break;

            case DX_MV1_VERTEX_TYPE_NMAP_4FRAME:
                shader = vertexShaderBoneNMap_;
                break;

            case DX_MV1_VERTEX_TYPE_8FRAME:
                shader = vertexShaderBone8Frame_;
                break;

            case DX_MV1_VERTEX_TYPE_NMAP_8FRAME:
                shader = vertexShaderBone8FrameNMap_;
                break;

            default:
                continue;
            }

            if (shader >= 0)
            {
                cache.lists.push_back({ i, shader });
            }
        }

        cache.initialized = true;
    }

    // キャッシュした情報で描画する
    for (const auto& list : cache.lists)
    {
        SetUseVertexShader(list.vertexShader);
        MV1DrawTriangleList(handle, list.index);
    }
}