#pragma once
#include "ActorBase.h"
#include "DrawableObject.h"

class JumpStand : public ActorBase
{
public:
	explicit JumpStand(std::string id) : ActorBase(std::move(id)) 
	{
		jumpStandModel_.SetOwner(*this);
		pillarModel_.SetOwner(*this);
	}
	virtual ~JumpStand() = default;
	void Init() override
	{
		Reset();
	}
	void Reset() override
	{

	}
	void Update(float) override {}
	void Draw() const override 
	{
		jumpStandModel_.Draw();
		pillarModel_.Draw();
	}

	ModelObject& GetJumpStandModel() { return jumpStandModel_; }
	const ModelObject& GetJumpStandModel() const { return jumpStandModel_; }
	ModelObject& GetPillarModel() { return pillarModel_; }
	const ModelObject& GetPillarModel() const { return pillarModel_; }
private:
	ModelObject jumpStandModel_;
	ModelObject pillarModel_;
};