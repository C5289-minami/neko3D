// =============================
// Scenes/Title/TitleScene.cpp
// =============================
#include "TitleScene.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "ResourceKeys.h"
#ifndef NDEBUG
#include "imgui.h"
#endif // !NDEBUG
#include "UIManager.h"


void TitleScene::Init()
{
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());

	DxLib::SetMouseDispFlag(TRUE);



	StartFadeIn();
}

void TitleScene::Update(float deltaTime)
{
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());
#ifndef NDEBUG
	if (DxLib::CheckHitKey(KEY_INPUT_F1))
	{
		SetNextScene(SceneManager::GetInstance().GetScene(SceneID::SsFlipTest));
		StartFadeOut();
		return;
	}
	else if (DxLib::CheckHitKey(KEY_INPUT_F2))
	{
		SetNextScene(SceneManager::GetInstance().GetScene(SceneID::ShaderTest));
		StartFadeOut();
		return;
	}
	else if (DxLib::CheckHitKey(KEY_INPUT_F3))
	{
		SetNextScene(SceneManager::GetInstance().GetScene(SceneID::NekoMotionTest));
		StartFadeOut();
		return;
	}
	else if (DxLib::CheckHitKey(KEY_INPUT_F4))
	{
		SetNextScene(SceneManager::GetInstance().GetScene(SceneID::StatusTest));
		StartFadeOut();
		return;
	}
	else if (DxLib::CheckHitKey(KEY_INPUT_F5))
	{
		SetNextScene(SceneManager::GetInstance().GetScene(SceneID::ModelConfig));
		StartFadeOut();
		return;
	}
	else if (DxLib::CheckHitKey(KEY_INPUT_F6))
	{
		SetNextScene(SceneManager::GetInstance().GetScene(SceneID::Game));
		StartFadeOut();
		return;
	}
	using namespace DxPlus::Input;
	if (GetButtonDown(PLAYER1) & BUTTON_START)
	{
		Scene* gameScene = nullptr;
		gameScene = SceneManager::GetInstance().GetScene(SceneID::GameTest);
		if (gameScene)
			SetNextScene(gameScene);
		StartFadeOut();
		return;
	}
#endif
}

void TitleScene::Render() const
{

}

#ifndef NDEBUG
void TitleScene::DrawImGui()
{
	const char* f1 = u8"F1: スプスタテストシーン";
	const char* f2 = u8"F2: シェーダーテストシーン";
	const char* f3 = u8"F3: 部屋テストシーン";
	const char* f4 = u8"F4: キャラテストシーン";
	const char* f5 = u8"F5: Model Config";
	const char* f6 = u8"F6: ゲームシーンへ遷移";
	const char* Enter = u8"Enter: ゲームTestシーンへ遷移";

	ImGui::Begin("TitleScene Debug Menu");

	ImGui::TextWrapped(f1);
	ImGui::TextWrapped(f2);
	ImGui::TextWrapped(f3);
	ImGui::TextWrapped(f4);
	ImGui::TextWrapped(f5);
	ImGui::TextWrapped(f6);
	ImGui::TextWrapped(Enter);

	ImGui::End();
}
#endif
