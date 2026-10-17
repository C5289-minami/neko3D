#pragma once
#include "ActorBase.h"
#include "Vector3.h"
#include "DrawableObject.h"

class Room : public ActorBase
{
public:
   Room();
	~Room() = default;

	void Init() override;
	void Reset() override;
	void Update(float deltaTime) override;
	void Draw() const override;

   ModelObject& GetModelObject() { return roomModel_; }
	const ModelObject& GetModelObject() const { return roomModel_; }
private:
	ModelObject roomModel_;
};

