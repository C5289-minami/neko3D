#pragma once
#include "ActorBase.h"
#include "Vector3.h"
#include "DrawableObject.h"
#include <vector>
#include "JumpStand.h"
#include "Basket.h"

class Room : public ActorBase
{
public:
	Room();
	~Room() = default;

	void Init() override;
	void Reset() override;
	void Update(float deltaTime) override;
	void Draw() const override;

	ModelObject& GetRoomObject() { return roomModel_; }
	const ModelObject& GetRoomObject() const { return roomModel_; }
	std::vector<ModelObject*> GetPillars() { return { &pillar1_, &pillar2_ }; }
	std::vector<const ModelObject*> GetPillars() const{ return { &pillar1_, &pillar2_ }; }
	ModelObject& GetChairObject() { return chairModel_; }
	std::vector<JumpStand*> GetJumpStands() { return { &lowJumpStand_, &mediumJumpStand_, &highJumpStand_ }; }
	std::vector<const JumpStand*> GetJumpStands() const { return { &lowJumpStand_, &mediumJumpStand_, &highJumpStand_ }; }
	Basket& GetBasket() { return basket_; }
	const Basket& GetBasket()const { return basket_; }

	const std::vector<const ModelObject*> GetAllModelObjects() const
	{
		std::vector<const ModelObject*> objects;
		objects.push_back(&roomModel_);
		objects.push_back(&pillar1_);
		objects.push_back(&pillar2_);
		objects.push_back(&chairModel_);
		objects.push_back(&lowJumpStand_.GetJumpStandModel());
		objects.push_back(&lowJumpStand_.GetPillarModel());
		objects.push_back(&mediumJumpStand_.GetJumpStandModel());
		objects.push_back(&mediumJumpStand_.GetPillarModel());
		objects.push_back(&highJumpStand_.GetJumpStandModel());
		objects.push_back(&highJumpStand_.GetPillarModel());
		objects.push_back(&basket_.GetModelObject());
		return objects;
	}

	std::vector <ModelObject*> GetAllModelObjects()
	{
		std::vector<ModelObject*> objects;
		objects.push_back(&roomModel_);
		objects.push_back(&pillar1_);
		objects.push_back(&pillar2_);
		objects.push_back(&chairModel_);
		objects.push_back(&lowJumpStand_.GetJumpStandModel());
		objects.push_back(&lowJumpStand_.GetPillarModel());
		objects.push_back(&mediumJumpStand_.GetJumpStandModel());
		objects.push_back(&mediumJumpStand_.GetPillarModel());
		objects.push_back(&highJumpStand_.GetJumpStandModel());
		objects.push_back(&highJumpStand_.GetPillarModel());
		objects.push_back(&basket_.GetModelObject());
		return objects;
	}

private:
	ModelObject roomModel_;
	ModelObject pillar1_;
	ModelObject pillar2_;
	ModelObject chairModel_;
	JumpStand lowJumpStand_;
	JumpStand mediumJumpStand_;
	JumpStand highJumpStand_;
	Basket basket_;
};

