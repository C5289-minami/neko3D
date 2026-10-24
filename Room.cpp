#include "Room.h"
#include "DxLib.h"
#include "Resourcekeys.h"
#include "ModelObjectManager.h"

// エラーを解消しつつ
// コピーせずに無理やりロードするかコピーを実装


Room::Room()
	: ActorBase("Room")
{
	roomModel_.SetModelKey(ResourceKeys::Model_Room);

	lowJumpStand_.jumpStandModel_.SetModelKey(
		ResourceKeys::Model_JumpStand);
	lowJumpStand_.pillarModel_.SetModelKey(
		ResourceKeys::Model_hashira_s);

	mediumJumpStand_.jumpStandModel_.SetModelKey(
		ResourceKeys::Model_JumpStand);
	mediumJumpStand_.pillarModel_.SetModelKey(
		ResourceKeys::Model_hashira_m);

	highJumpStand_.jumpStandModel_.SetModelKey(
		ResourceKeys::Model_JumpStand);
	highJumpStand_.pillarModel_.SetModelKey(
		ResourceKeys::Model_hashira_l);

	// 部屋
	roomModel_.SetObjectId("room_model");
	roomModel_.SetObjectName("Room");

	// 低いジャンプ台
	lowJumpStand_.jumpStandModel_.SetObjectId("low_jump_stand");
	lowJumpStand_.jumpStandModel_.SetObjectName("Low Jump Stand");
	lowJumpStand_.pillarModel_.SetObjectId("low_jump_stand_pillar");
	lowJumpStand_.pillarModel_.SetObjectName("Low Jump Stand Pillar");

	// 中くらいのジャンプ台
	mediumJumpStand_.jumpStandModel_.SetObjectId("medium_jump_stand");
	mediumJumpStand_.jumpStandModel_.SetObjectName("Medium Jump Stand");
	mediumJumpStand_.pillarModel_.SetObjectId("medium_jump_stand_pillar");
	mediumJumpStand_.pillarModel_.SetObjectName("Medium Jump Stand Pillar");

	// 高いジャンプ台
	highJumpStand_.jumpStandModel_.SetObjectId("high_jump_stand");
	highJumpStand_.jumpStandModel_.SetObjectName("High Jump Stand");
	highJumpStand_.pillarModel_.SetObjectId("high_jump_stand_pillar");
	highJumpStand_.pillarModel_.SetObjectName("High Jump Stand Pillar");
}
void Room::Init()
{
	Reset();

}

void Room::Reset()
{
	MObjs().LoadActorTransform(*this);

	roomModel_.Load();

	lowJumpStand_.jumpStandModel_.Load();
	lowJumpStand_.pillarModel_.Load();

	mediumJumpStand_.jumpStandModel_.Load();
	mediumJumpStand_.pillarModel_.Load();

	highJumpStand_.jumpStandModel_.Load();
	highJumpStand_.pillarModel_.Load();
}

void Room::Update(float deltaTime)
{
	(void)deltaTime; // 未使用の引数を無視
}

void Room::Draw() const
{
	roomModel_.Draw();
}
