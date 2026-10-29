#pragma once
#include "ModelType.h"
#include <unordered_map>
#include <vector>




struct ModelObject;

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
    bool Init();

    // 確保したリソースを解放する
    void Release() noexcept;

    // トゥーン描画を開始する
    // 設定値をGPUへ反映し、ピクセルシェーダーを設定する
    void BeginPass() const;

    // トゥーン描画を終了する
    void EndPass() const;

    // 指定したモデルをトゥーン描画する
    void Draw(const ModelObject& model) const;
  
    // 深度パス用の描画。オブジェクトIDは自動割り当て
    void DrawDepth(const ModelObject& model) const;
    // オブジェクトIDを指定して深度パス用に描画
    void DrawDepth(const ModelObject& model, int objectId) const;

    // 頂点シェーダーの設定
    // モデルタイプに応じた頂点シェーダーを設定する
    void SetVertexShader(ModelType type) const;

private:
    // 頂点タイプを判定し、リストごとに描画する
    void DrawTriangleLists(const ModelObject& model) const;

    // トゥーンの色を計算するピクセルシェーダー
    int pixelShader_{ -1 };
    // 通常モデル用の頂点シェーダー
    int vertexShader_{ -1 };
    // ボーン変形対応の頂点シェーダー
    int vertexShaderBone_{ -1 };
    // ボーン変形＋法線マップ対応の頂点シェーダー
    int vertexShaderBoneNMap_{ -1 };
    // 8FRAME対応の頂点シェーダー
    int vertexShaderBone8Frame_{ -1 };
    int vertexShaderBone8FrameNMap_{ -1 };
    // トゥーン設定をGPUへ渡すための定数バッファー
    int toonConstantBuffer_{ -1 };

    struct DepthObjectIdConstants
    {
        float objectId = 1.0f;
        float padding[3]{};
    };

    // オブジェクトごとのIDを管理
    mutable std::unordered_map<const ModelObject*, int> depthObjectIds_;
    mutable int nextDepthObjectId_{ 1 };
    int depthObjectIdConstantBuffer_{ -1 };

    struct TriangleListInfo
    {
        int index;
        int vertexShader;
    };

    struct TriangleListCache
    {
        bool initialized = false;
        std::vector<TriangleListInfo> lists;
    };
    mutable std::unordered_map<int, TriangleListCache> triangleListsCache_;
};
