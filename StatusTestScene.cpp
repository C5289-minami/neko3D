#include "StatusTestScene.h"
#include "GameContext.h"

void StatusTestScene::Init()
{
	// ”wŒiF
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());

	camera_.Reset();
	freeCamera_.Reset();
	activeCamera_ = &freeCamera_;
	activeCamera_->ApplyCamera();
}

void StatusTestScene::Update(float deltaTime)
{
	// ”wŒiF
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());

	activeCamera_->Update(deltaTime);
}

void StatusTestScene::Render() const
{
	activeCamera_->ApplyCamera();

	DxLib::DrawBillboard3DToShader(
		DxLib::VECTOR{ 0.0f, 0.0f, 0.0f }, // Pos
		1.0f, 1.0f, // cx, cy
		1.0f, // Size
		0.0f, // Angle
		-1, // GrHandle
		FALSE, // TransFlag
		FALSE, // ReverseXFlag
		FALSE // ReverseYFlag
	);
}
