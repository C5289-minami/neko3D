#pragma once

#include "DrawableObject.h"

#include <vector>

// モデル編集用のオブジェクト管理クラス

class ModelObjectManager
{
public:
    static ModelObjectManager& GetInstance();

	// オブジェクトの追加・削除・描画・保存
    void Add(ModelObject& object);
    void Remove(ModelObject& object);
    void Draw() const;
    bool SaveAll() const;
    const std::vector<ModelObject*>& GetObjects() const { return objects_; }

private:
    std::vector<ModelObject*> objects_;
};

// シングルトン
inline ModelObjectManager& MObjs() { return ModelObjectManager::GetInstance(); }
