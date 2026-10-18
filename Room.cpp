#include "Room.h"
#include "DxLib.h"
#include "Resourcekeys.h"
#include "ModelObjectManager.h"

Room::Room()
	: ActorBase("Room")
{
	roomModel_.modelKey = ResourceKeys::Model_Room;
}

void Room::Init()
{
	Reset();
	roomModel_.Init();
}

void Room::Reset()
{
 MObjs().LoadActorTransform(*this);
	float scale = 1.0f;
	roomModel_.scale = { scale, scale, scale };
}

void Room::Update(float deltaTime)
{
	(void)deltaTime; // ñ¢égópÇÃà¯êîÇñ≥éã
}

void Room::Draw() const
{
	roomModel_.Draw();
}
