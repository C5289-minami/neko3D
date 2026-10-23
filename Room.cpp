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

	mediumJumpStand_.jumpStandModel_.SetModelKey(
		ResourceKeys::Model_JumpStand);

	highJumpStand_.jumpStandModel_.SetModelKey(
		ResourceKeys::Model_JumpStand);
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
