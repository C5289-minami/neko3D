#include "Room.h"
#include "DxLib.h"
#include "Resourcekeys.h"
#include "ModelObjectManager.h"



Room::Room()
	: ActorBase("Room"),
	lowJumpStand_("LowJumpStand"),
	mediumJumpStand_("MediumJumpStand"),
	highJumpStand_("HighJumpStand")
{
	roomModel_.SetModelKey(ResourceKeys::Model_Room);
	pillar1_.SetModelKey(ResourceKeys::Model_PillarM);
	pillar2_.SetModelKey(ResourceKeys::Model_PillarM);
	chairModel_.SetModelKey(ResourceKeys::Model_Chair);

	for (auto& stand: this->GetJumpStands())
	{
		stand->GetJumpStandModel().SetModelKey(
			ResourceKeys::Model_JumpStand);
		stand->GetPillarModel().SetModelKey(
			ResourceKeys::Model_PillarM);

		stand->SetOwner(*this);

		stand->Init();
	}

	roomModel_.SetOwner(*this);
	pillar1_.SetOwner(*this);
	pillar2_.SetOwner(*this);
	chairModel_.SetOwner(*this);
	basket_.SetOwner(*this);

	// 部屋
	roomModel_.SetObjectId("room_model");
	roomModel_.SetObjectName("Room");

	pillar1_.SetObjectId("room_pillar1");
	pillar2_.SetObjectId("room_pillar2");
	chairModel_.SetObjectId("room_chair");
	pillar1_.SetObjectName("Room Pillar1");
	pillar2_.SetObjectName("Room Pillar2");
	chairModel_.SetObjectName("Room Chair");


	// 低いジャンプ台
	lowJumpStand_.GetJumpStandModel().SetObjectId("low_jump_stand");
	lowJumpStand_.GetJumpStandModel().SetObjectName("Low Jump Stand");
	lowJumpStand_.GetPillarModel().SetObjectId("low_jump_stand_pillar");
	lowJumpStand_.GetPillarModel().SetObjectName("Low Jump Stand Pillar");

	// 中くらいのジャンプ台
	mediumJumpStand_.GetJumpStandModel().SetObjectId("medium_jump_stand");
	mediumJumpStand_.GetJumpStandModel().SetObjectName("Medium Jump Stand");
	mediumJumpStand_.GetPillarModel().SetObjectId("medium_jump_stand_pillar");
	mediumJumpStand_.GetPillarModel().SetObjectName("Medium Jump Stand Pillar");

	// 高いジャンプ台
	highJumpStand_.GetJumpStandModel().SetObjectId("high_jump_stand");
	highJumpStand_.GetJumpStandModel().SetObjectName("High Jump Stand");
	highJumpStand_.GetPillarModel().SetObjectId("high_jump_stand_pillar");
	highJumpStand_.GetPillarModel().SetObjectName("High Jump Stand Pillar");


}
void Room::Init()
{
	Reset();

}

void Room::Reset()
{
	MObjs().LoadActorTransform(*this);

	roomModel_.Load();
	pillar1_.Load();
	pillar2_.Load();
	chairModel_.Load();
	
	MObjs().LoadActorTransform(basket_);

	for (auto& stand : this->GetJumpStands())
	{
		MObjs().LoadActorTransform(*stand);
		stand->GetJumpStandModel().Load();
		stand->GetPillarModel().Load();
	}
}

void Room::Update(float deltaTime)
{
	(void)deltaTime; // 未使用の引数を無視
}

void Room::Draw() const
{
	roomModel_.Draw();
	pillar1_.Draw();
	pillar2_.Draw();
	chairModel_.Draw();

	lowJumpStand_.Draw();
	mediumJumpStand_.Draw();
	highJumpStand_.Draw();

	basket_.Draw();
}
