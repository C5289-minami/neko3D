#pragma once
#include "ActorBase.h"
#include "Vector3.h"
#include "DrawableObject.h"
#include "TransformSettings.h"

class Room : public ActorBase
{
public:
	Room() = default;
	~Room() = default;

	void Init() override;
	void Reset() override;
	void Update(float deltaTime) override;
	void Draw() const override;

	ModelObject GetModelObject() const { return roomModel_; }
    ObjectTransform GetTransform() const
    {
        return { roomModel_.scale, roomModel_.position, roomModel_.rotation };
    }
    void SetTransform(const ObjectTransform& value)
    {
        roomModel_.scale = value.scale;
        roomModel_.position = value.position;
        roomModel_.rotation = value.rotation;
    }
private:
	ModelObject roomModel_;
};

