#include "NekoMotionTestScene.h"

void NekoMotionTestScene::Init()
{
	player_.Init();
	room_.Init();
}

void NekoMotionTestScene::Update(float deltaTime)
{
	player_.Update(deltaTime);
	room_.Update(deltaTime);
}

void NekoMotionTestScene::Render() const
{
	player_.Draw();
	room_.Draw();
}
