#pragma once
#include "ActorBase.h"
#include "Vector3.h"
#include "DrawableObject.h"


class Room : public ActorBase
{
public:
	Room() = default;
	~Room() = default;

	void Init() override;
	void Reset() override;
	void Update(float deltaTime) override;
	void Draw() const override;
private:
	ModelObject roomModel_;

};

