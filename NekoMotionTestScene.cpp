#include "NekoMotionTestScene.h"

void NekoMotionTestScene::Init()
{
	player_.Init();
	room_.Init();
	debugCamera_.Initialize({ 0.0f, 400.0f, -400.0f }, { 0.0f, 100.0f, 0.0f });
}

void NekoMotionTestScene::Update(float deltaTime)
{
	player_.Update(0);
	room_.Update(deltaTime);

	debugCamera_.Update(deltaTime, player_.GetPosition(), debugCamera_.GetEye(), debugCamera_.GetTarget());
}

void NekoMotionTestScene::Render() const
{
	if (debugCamera_.IsSceneViewActive()) debugCamera_.ApplyCamera();
	player_.Draw();
	room_.Draw();
	grid_.Draw();
}
