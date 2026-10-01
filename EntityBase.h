#pragma once
#include "Vector3.h"
#include "ActorBase.h"

class EntityBase : public ActorBase
{
	public:
	EntityBase() = default;
	virtual ~EntityBase() = default;
	virtual void Init() override = 0;
	virtual void Reset() override = 0;
	virtual void Update(float deltaTime) override = 0;
	virtual void Draw() const override = 0;


	// ゲッターセッター
private:
	int maxHp{ 100 };
	int currentHp{ 100 };

};