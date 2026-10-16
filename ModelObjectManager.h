#pragma once

#include "DrawableObject.h"

#include <vector>

class ModelObjectManager
{
public:
    static ModelObjectManager& GetInstance();

    void Add(ModelObject& object);
    void Remove(ModelObject& object);
    void Draw() const;
    bool SaveAll() const;
    const std::vector<ModelObject*>& GetObjects() const { return objects_; }

private:
    std::vector<ModelObject*> objects_;
};

inline ModelObjectManager& MObjs() { return ModelObjectManager::GetInstance(); }
