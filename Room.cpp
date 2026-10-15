#include "Room.h"
#include "DxLib.h"
#include "Resourcekeys.h"

void Room::Init()
{
	roomModel_.modelKey = ResourceKeys::Model_Room;
	Reset();
}

void Room::Reset()
{
	float scale = 1.0f;
	roomModel_.scale = { scale, scale, scale };
}

void Room::Update(float deltaTime)
{
}

void Room::Draw() const
{
	roomModel_.Draw();
}
