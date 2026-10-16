#include "ModelObjectManager.h"

#include <algorithm>

ModelObjectManager& ModelObjectManager::GetInstance()
{
    static ModelObjectManager instance;
    return instance;
}

void ModelObjectManager::Add(ModelObject& object)
{
    if (std::find(objects_.begin(), objects_.end(), &object) != objects_.end()) return;
    object.Init();
    objects_.push_back(&object);
}

void ModelObjectManager::Remove(ModelObject& object)
{
    objects_.erase(std::remove(objects_.begin(), objects_.end(), &object), objects_.end());
}

void ModelObjectManager::Draw() const
{
    for (const auto* object : objects_)
        if (object) object->Draw();
}

bool ModelObjectManager::SaveAll() const
{
    bool success = true;
    for (const auto* object : objects_)
        if (object) success = object->Save() && success;
    return success;
}
