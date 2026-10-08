#include "ModelToonRenderer.h"

#include "DrawableObject.h"
#include "DxLib.h"

void ModelToonRenderer::Init()
{
    pixelShader_ =
        LoadPixelShader(
            L"./DevData/ShaderCompiler/Bin/ModelToonPS.pso"
        );

    vertexShader4Frame_ =
        LoadVertexShader(
            L"./DevData/ShaderCompiler/Bin/ModelToonVS_4Frame.vso"
        );

    vertexShaderNMap4Frame_ =
        LoadVertexShader(
            L"./DevData/ShaderCompiler/Bin/ModelToonVS_NMap4Frame.vso"
        );
}

void ModelToonRenderer::Draw(const ModelObject& model, ModelToonType type) const
{
    SetUsePixelShader(pixelShader_);

    switch (type)
    {
    case ModelToonType::FourFrame:
        SetUseVertexShader(vertexShader4Frame_);
        break;

    case ModelToonType::NMap4Frame:
        SetUseVertexShader(vertexShaderNMap4Frame_);
        break;
    }
    model.Draw();
}

void ModelToonRenderer::SetVertexShader(ModelToonType type) const
{
    switch (type)
    {
    case ModelToonType::FourFrame:
        SetUseVertexShader(vertexShader4Frame_);
        break;

    case ModelToonType::NMap4Frame:
        SetUseVertexShader(vertexShaderNMap4Frame_);
        break;
    }
}