#pragma once
#include "ActorBase.h"
#include "Vector3.h"
#include "DrawableObject.h"
#include <vector>
#include "JumpStand.h"

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
	std::vector<JumpStand*> GetJumpStands() { return { &lowJumpStand_, &mediumJumpStand_, &highJumpStand_ }; }

private:
	ModelObject roomModel_;
	ModelObject pillar1_;
	ModelObject pillar2_;
	JumpStand lowJumpStand_;
	JumpStand mediumJumpStand_;
	JumpStand highJumpStand_;
};

