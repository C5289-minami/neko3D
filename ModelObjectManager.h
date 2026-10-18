#pragma once

#include "DrawableObject.h"
#include "ActorBase.h"

#include <string>
#include <utility>
#include <vector>

// モデル編集用のオブジェクト管理クラス

class ModelObjectManager
{
public:
    static ModelObjectManager& GetInstance();

	// オブジェクトの追加・削除・描画・保存
    void Add(ModelObject& object);
    void Add(ModelObject& object, ActorBase& owner);
    void Remove(ModelObject& object);
    void AddActor(const std::string& name, ActorBase& actor);
    ActorBase* GetOwner(const ModelObject& object) const;
    void Draw() const;
    bool SaveAll() const;
    bool LoadActorTransform(ActorBase& actor,
        const std::string& filePath = "./Data/Config/model_objects.json") const;
    bool SaveActorTransform(const ActorBase& actor,
        const std::string& filePath = "./Data/Config/model_objects.json") const;
    const std::vector<std::pair<std::string, ActorBase*>>& GetActors() const { return actors_; }
    const std::vector<ModelObject*>& GetObjects() const { return objects_; }

private:
    std::vector<ModelObject*> objects_;
    std::vector<std::pair<std::string, ActorBase*>> actors_;
    std::vector<std::pair<ModelObject*, ActorBase*>> ownership_;
};

// シングルトン
inline ModelObjectManager& MObjs() { return ModelObjectManager::GetInstance(); }
