#include "ScreenSpaceOutlineRenderer.h"

#include "OutlineSettings.h"

namespace
{
    constexpr int OutlineSettingsSlot = 6;
    constexpr const wchar_t* DepthShaderPath =
        L"./DevData/ShaderCompiler/Bin/DepthPS.pso";
    constexpr const wchar_t* PostProcessShaderPath =
        L"./DevData/ShaderCompiler/Bin/PostProcessPS.pso";

    constexpr const wchar_t* FullScreenPixelShaderPath =
		L"./DevData/ShaderCompiler/Bin/FullScreenPS.pso";
}

ScreenSpaceOutlineRenderer::~ScreenSpaceOutlineRenderer()
{
    Release();
}

bool ScreenSpaceOutlineRenderer::Init(int width, int height)
{
    Release();

    if (width <= 0 || height <= 0)
    {
        return false;
    }

    width_ = width;
    height_ = height;

    depthPixelShader_ = LoadPixelShader(DepthShaderPath);
    postProcessPixelShader_ = LoadPixelShader(PostProcessShaderPath);
    fullScreenPixelShader_ = LoadPixelShader(FullScreenPixelShaderPath);

    // 深度・法線を RGBA32F で保持できる画面を作る。
    SetDrawValidFloatTypeGraphCreateFlag(TRUE);
    SetCreateDrawValidGraphChannelNum(4);
    SetCreateGraphChannelBitDepth(32);

    depthBuffer_ = MakeScreen(width_, height_, TRUE);
    sceneBuffer_ = MakeScreen(width_, height_, TRUE);

    // 以降に作る通常の画面へ設定が漏れないよう、必ず元へ戻す。
    SetDrawValidFloatTypeGraphCreateFlag(FALSE);
    SetCreateDrawValidGraphChannelNum(0);
    SetCreateGraphChannelBitDepth(0);

    // アウトライン適用後の画像を保存するバッファ
    outlineBuffer_ = MakeScreen(width_, height_, TRUE);

    if (sceneBuffer_ >= 0)
    {
        SetUseGraphZBuffer(sceneBuffer_, TRUE);
    }
    if (depthBuffer_ >= 0)
    {
        SetUseGraphZBuffer(depthBuffer_, TRUE);
    }

    settingsConstantBuffer_ =
        CreateShaderConstantBuffer(sizeof(OutlineSettings));

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

    auto* settings = static_cast<OutlineSettings*>(
        GetBufferShaderConstantBuffer(settingsConstantBuffer_));
    if (settings == nullptr)
    {
        Release();
        return false;
    }

    *settings = g_outlineSettings;
    UpdateShaderConstantBuffer(settingsConstantBuffer_);

  

    CreateFullscreenVertices();
    return true;
}

void ScreenSpaceOutlineRenderer::Release() noexcept
{
    if (settingsConstantBuffer_ >= 0)
    {
        DeleteShaderConstantBuffer(settingsConstantBuffer_);
        settingsConstantBuffer_ = -1;
    }

    if (postProcessPixelShader_ >= 0)
    {
        DeleteShader(postProcessPixelShader_);
        postProcessPixelShader_ = -1;
    }

    if (depthPixelShader_ >= 0)
    {
        DeleteShader(depthPixelShader_);
        depthPixelShader_ = -1;
    }

    if (sceneBuffer_ >= 0)
    {
        DeleteGraph(sceneBuffer_);
        sceneBuffer_ = -1;
    }

    if (depthBuffer_ >= 0)
    {
        DeleteGraph(depthBuffer_);
        depthBuffer_ = -1;
    }

    if (fullScreenPixelShader_ >= 0)
    {
        DeleteShader(fullScreenPixelShader_);
        fullScreenPixelShader_ = -1;
    }
    if (outlineBuffer_ >= 0)
    {
        DeleteGraph(outlineBuffer_);
        outlineBuffer_ = -1;
    }

    width_ = 0;
    height_ = 0;
}

void ScreenSpaceOutlineRenderer::BeginScenePass() const
{
    if (sceneBuffer_ < 0)
    {
        return;
    }

    SetDrawScreen(sceneBuffer_);
    SetDrawZBuffer(sceneBuffer_);
    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(TRUE);
    MV1SetUseOrigShader(TRUE);

    ClearDrawScreen();
    ClearDrawScreenZBuffer();
}

void ScreenSpaceOutlineRenderer::EndScenePass() const
{
    SetUsePixelShader(-1);
    SetUseVertexShader(-1);
    MV1SetUseOrigShader(FALSE);
    SetUseZBuffer3D(FALSE);
    SetWriteZBuffer3D(FALSE);
    SetDrawZBuffer(-1);
    SetDrawScreen(DX_SCREEN_BACK);
}

void ScreenSpaceOutlineRenderer::BeginDepthPass() const
{
    if (depthBuffer_ < 0)
    {
        return;
    }

    SetDrawScreen(depthBuffer_);
    SetDrawZBuffer(depthBuffer_);
    SetUseZBuffer3D(TRUE);
    SetWriteZBuffer3D(TRUE);
    MV1SetUseOrigShader(TRUE);

    ClearDrawScreen();
    ClearDrawScreenZBuffer();
    SetUsePixelShader(depthPixelShader_);
}

void ScreenSpaceOutlineRenderer::EndDepthPass() const
{
    SetUsePixelShader(-1);
    SetUseVertexShader(-1);
    MV1SetUseOrigShader(FALSE);
    SetUseZBuffer3D(FALSE);
    SetWriteZBuffer3D(FALSE);
    SetDrawZBuffer(-1);
    SetDrawScreen(DX_SCREEN_BACK);
}

void ScreenSpaceOutlineRenderer::RenderPostProcess() const
{
    if (sceneBuffer_ < 0)
    {
        return;
    }

    if (outlineEnabled_ &&
        (depthBuffer_ < 0 ||
            postProcessPixelShader_ < 0 ||
            settingsConstantBuffer_ < 0))
    {
        return;
    }

    if (fullScreenEffectEnabled_ &&
        fullScreenPixelShader_ < 0)
    {
        return;
    }

    if (outlineEnabled_ && fullScreenEffectEnabled_)
    {
        // ① シーン画像にハーフトーンなどの画面効果を適用
        // sceneBuffer_ → outlineBuffer_
        if (outlineBuffer_ < 0)
        {
            return;
        }

        RenderFullScreenPass(
            sceneBuffer_,
            outlineBuffer_
        );

        // ② 効果適用後の画像にアウトラインを描画
        // outlineBuffer_ → バックバッファ
        RenderOutlinePass(
            outlineBuffer_,
            DX_SCREEN_BACK
        );
    }
    else if (outlineEnabled_)
    {
        // アウトラインだけ
        RenderOutlinePass(
            sceneBuffer_,
            DX_SCREEN_BACK
        );
    }
    else if (fullScreenEffectEnabled_)
    {
        // 全画面エフェクトだけ
        RenderFullScreenPass(
            sceneBuffer_,
            DX_SCREEN_BACK
        );
    }
    else
    {
        // 両方無効
        SetDrawScreen(DX_SCREEN_BACK);
        SetDrawZBuffer(-1);

        DrawGraph(0, 0, sceneBuffer_, FALSE);
    }
}
void ScreenSpaceOutlineRenderer::RenderOutlinePass(
    int sourceTexture,
    int destinationScreen) const
{
    if (sourceTexture < 0 ||
        destinationScreen < 0 && destinationScreen != DX_SCREEN_BACK)
    {
        return;
    }

    auto* settings = static_cast<OutlineSettings*>(
        GetBufferShaderConstantBuffer(settingsConstantBuffer_));

    if (settings == nullptr)
    {
        return;
    }

    *settings = g_outlineSettings;
    UpdateShaderConstantBuffer(settingsConstantBuffer_);

    SetDrawScreen(destinationScreen);
    SetDrawZBuffer(-1);

    SetShaderConstantBuffer(
        settingsConstantBuffer_,
        DX_SHADERTYPE_PIXEL,
        OutlineSettingsSlot
    );

    // 入力画像を指定できるようにする
    SetUseTextureToShader(0, sourceTexture);
    SetUseTextureToShader(1, depthBuffer_);
    SetUsePixelShader(postProcessPixelShader_);

    DrawPrimitive2DToShader(
        const_cast<VERTEX2DSHADER*>(fullscreenVertices_),
        6,
        DX_PRIMTYPE_TRIANGLELIST
    );

    SetUsePixelShader(-1);
    SetUseTextureToShader(0, -1);
    SetUseTextureToShader(1, -1);

    SetDrawScreen(DX_SCREEN_BACK);
}
bool ScreenSpaceOutlineRenderer::IsInitialized() const noexcept
{
    return depthPixelShader_ >= 0 &&
        postProcessPixelShader_ >= 0 &&
        fullScreenPixelShader_ >= 0 &&
        sceneBuffer_ >= 0 &&
        depthBuffer_ >= 0 &&
        outlineBuffer_ >= 0 &&
        settingsConstantBuffer_ >= 0;
}

void ScreenSpaceOutlineRenderer::RenderFullScreenShader() const
{
    if (sceneBuffer_ < 0 || fullScreenPixelShader_ < 0)
    {
        return;
    }

    // 出力先をバックバッファにする
    SetDrawScreen(DX_SCREEN_BACK);
    SetDrawZBuffer(-1);

    // シーン画像をシェーダーに渡す
    SetUseTextureToShader(0, sceneBuffer_);

    // 全画面エフェクトを適用
    SetUsePixelShader(fullScreenPixelShader_);

    DrawPrimitive2DToShader(
        const_cast<VERTEX2DSHADER*>(fullscreenVertices_),
        6,
        DX_PRIMTYPE_TRIANGLELIST
    );

    // 描画状態を解除
    SetUsePixelShader(-1);
    SetUseTextureToShader(0, -1);
    SetUseTextureToShader(1, -1);
}
void ScreenSpaceOutlineRenderer::RenderFullScreenPass(
    int sourceTexture,
    int destinationScreen) const
{
    if (sourceTexture < 0 ||
        fullScreenPixelShader_ < 0)
    {
        return;
    }

    SetDrawScreen(destinationScreen);
    SetDrawZBuffer(-1);

    SetUseTextureToShader(0, sourceTexture);
    SetUsePixelShader(fullScreenPixelShader_);

    DrawPrimitive2DToShader(
        const_cast<VERTEX2DSHADER*>(fullscreenVertices_),
        6,
        DX_PRIMTYPE_TRIANGLELIST
    );

    SetUsePixelShader(-1);
    SetUseTextureToShader(0, -1);
    SetUseTextureToShader(1, -1);

    SetDrawScreen(DX_SCREEN_BACK);
}   
void ScreenSpaceOutlineRenderer::CreateFullscreenVertices()
{
    const float width = static_cast<float>(width_);
    const float height = static_cast<float>(height_);

    const VECTOR positions[6] =
    {
        VGet(0.0f, 0.0f, 0.0f),
        VGet(width, 0.0f, 0.0f),
        VGet(0.0f, height, 0.0f),
        VGet(width, height, 0.0f),
        VGet(0.0f, height, 0.0f),
        VGet(width, 0.0f, 0.0f)
    };

    const float uvs[6][2] =
    {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 0.0f, 1.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },
        { 1.0f, 0.0f }
    };

    for (int i = 0; i < 6; ++i)
    {
        fullscreenVertices_[i].pos = positions[i];
        fullscreenVertices_[i].rhw = 1.0f;
        fullscreenVertices_[i].dif = GetColorU8(255, 255, 255, 255);
        fullscreenVertices_[i].spc = GetColorU8(0, 0, 0, 0);
        fullscreenVertices_[i].u = uvs[i][0];
        fullscreenVertices_[i].v = uvs[i][1];
        fullscreenVertices_[i].su = uvs[i][0];
        fullscreenVertices_[i].sv = uvs[i][1];
    }
}
