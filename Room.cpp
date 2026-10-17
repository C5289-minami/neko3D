#include "Room.h"
#include "DxLib.h"
#include "Resourcekeys.h"

Room::Room()
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
	float scale = 1.0f;
	roomModel_.scale = { scale, scale, scale };
}

void Room::Update(float deltaTime)
{
	(void)deltaTime; // –¢g—p‚Ìˆø”‚ğ–³‹
}

void Room::Draw() const
{
	roomModel_.Draw();
}
