#pragma once

struct ModelObject;

// モデルの種類
enum class ModelToonType
{
    Model,  // 通常モデル
    Bone    // ボーン変形を使うモデル
};

// モデルのトゥーン描画を管理するクラス
class ModelToonRenderer
{
public:
    ModelToonRenderer() = default;
    ~ModelToonRenderer();

    // GPUリソースをコピーして二重解放する事故を防ぐ
    ModelToonRenderer(const ModelToonRenderer&) = delete;
    ModelToonRenderer& operator=(const ModelToonRenderer&) = delete;

    // シェーダーと定数バッファーを初期化する
    void Init();

    // 確保したリソースを解放する
    void Release() noexcept;

    // トゥーン描画を開始する
    // 設定値をGPUへ反映し、ピクセルシェーダーを設定する
    void BeginPass() const;

    // トゥーン描画を終了する
    void EndPass() const;

    // 指定したモデルをトゥーン描画する
    void Draw(
        const ModelObject& model,
        ModelToonType type
    ) const;

    // 頂点シェーダーだけを切り替える
    // 深度・法線の描画中にも使用する
    void SetVertexShader(ModelToonType type) const;

private:
    // トゥーンの色を計算するピクセルシェーダー
    int pixelShader_{ -1 };

    // 通常モデル用の頂点シェーダー
    int vertexShader_{ -1 };

    // ボーン変形対応の頂点シェーダー
    int vertexShaderBone_{ -1 };

    // トゥーン設定をGPUへ渡すための定数バッファー
    int toonConstantBuffer_{ -1 };
};
