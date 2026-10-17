#include "ModelObjectManager.h"

#include <algorithm>

ModelObjectManager& ModelObjectManager::GetInstance()
{
    static ModelObjectManager instance;
    return instance;
}

// オブジェクトの追加・削除・描画・保存
void ModelObjectManager::Add(ModelObject& object)
{
	// すでに追加されている場合は何もしない
    if (std::find(objects_.begin(), objects_.end(), &object) != objects_.end()) return;
	// 初期化して追加
    object.Init();
    objects_.push_back(&object);
}

void ModelObjectManager::Remove(ModelObject& object)
{
	// オブジェクトを削除
    objects_.erase(std::remove(objects_.begin(), objects_.end(), &object), objects_.end());
}

void ModelObjectManager::Draw() const
{
	// 登録されているオブジェクトを描画
    for (const auto* object : objects_)
        if (object) object->Draw();
}

bool ModelObjectManager::SaveAll() const
{
	// 登録されているオブジェクトをすべて保存
    bool success = true;
    for (const auto* object : objects_)
        if (object) success = object->Save() && success;
    return success;
}
