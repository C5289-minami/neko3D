// =============================
// Gameplay/Actors/Stage.cpp
// =============================
#include "Stage.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "DxConv.h"
#include "Raycast.h"

void Stage::Init()
{
	modelHandle = RM().GetModel(ResourceKeys::Model_Stage);
	if (modelHandle < 0) return;
	// ƒRƒŠƒWƒ‡ƒ“î•ñ‚ð¶¬
	MV1SetupCollInfo(modelHandle, -1, 8, 8, 8);
}

void Stage::Reset()
{
}

void Stage::Draw() const
{
	if (modelHandle < 0) return;
	// Œ³ƒ‚ƒfƒ‹
	MV1DrawModel(modelHandle);
}