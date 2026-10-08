#pragma once

struct ModelObject;

enum class ModelToonType
{
    FourFrame,
    NMap4Frame
};

class ModelToonRenderer
{
public:
    void Init();

    void Draw(
        const ModelObject& model,
        ModelToonType type
    ) const;

    void SetVertexShader(ModelToonType type) const;
private:
    int pixelShader_ = -1;
    int vertexShader4Frame_ = -1;
    int vertexShaderNMap4Frame_ = -1;
};