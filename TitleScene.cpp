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


	// UIの作成
	{
		// キャラクターのスプライトスタジオUIを作成
		{
			const std::wstring key = ResourceKeys::SpriteStudio_TitleCharacter;
			const DxPlus::Vec2 testPos(1000, 500);
			const DxPlus::Vec2 fromBelow(0.8f, 0.2f);
			const float motionDuration = 1.0f;
			auto* testUI = UIM().CreateSpriteStudioUI(
				L"Title.Character", key, testPos, "walk", fromBelow, motionDuration);
			if (testUI)
			{
				testUI->SetShowEasing(Motion::Easing::EaseOut);
				testUI->SetMoveDistance(1000.0f);
				testUI->SetScale(0.5f);
			}
		}
		// ロゴのUIを作成
		{
			const std::wstring key = ResourceKeys::UI_Logo;
			const DxPlus::Vec2 logoPos(DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.3f);
			const DxPlus::Vec2 fromAbove(0.0f, -1.0f);
			const float motionDuration = 1.0f;
			auto* logoUI = UIM().CreateImageUI(L"Title.Logo", key, logoPos, fromAbove, motionDuration);
			if (logoUI)
			{
				logoUI->SetShowEasing(Motion::Easing::EaseOut);
				logoUI->SetMoveDistance(500.0f);
				logoUI->SetScale(1.0f);
			}
		}
		// スタートボタンのUIを作成
		{
			const std::wstring key = ResourceKeys::UI_StartButton;
			const DxPlus::Vec2 startPos(DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.6f);
			const DxPlus::Vec2 fromBelow(0.0f, 1.0f);
			const float motionDuration = 1.0f;
			auto* startButtonUI = UIM().CreateButtonUI(
				L"Title.StartButton", key, startPos,
              {}, fromBelow, motionDuration);
			if (startButtonUI)
			{
				startButtonUI->SetShowEasing(Motion::Easing::EaseOut);
				startButtonUI->SetMoveDistance(500.0f);
				startButtonUI->SetScale(1.0f);
			}
		}
		// 終了ボタンのUIを作成
		{
			const std::wstring key = ResourceKeys::UI_ExitButton;
			const DxPlus::Vec2 exitPos(DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.75f);
			const DxPlus::Vec2 fromBelow(0.0f, 1.0f);
			const float motionDuration = 1.0f;
			auto* exitButtonUI = UIM().CreateButtonUI(
				L"Title.ExitButton", key, exitPos,
				[]() { PostQuitMessage(0); }, fromBelow, motionDuration);
			if (exitButtonUI)
			{
				exitButtonUI->SetShowEasing(Motion::Easing::EaseOut);
				exitButtonUI->SetMoveDistance(500.0f);
				exitButtonUI->SetScale(1.0f);
			}
		}
	}

	StartFadeIn();
}

void TitleScene::Update(float deltaTime)
{
	DxLib::SetBackgroundColor(gameContext->GetBgRed(), gameContext->GetBgGreen(), gameContext->GetBgBlue());
  if (auto* startButtonUI = UIM().GetButtonUI(L"Title.StartButton");
		startButtonUI && startButtonUI->IsPressed())
	{
		SetNextScene(SceneManager::GetInstance().GetScene(SceneID::Game));
		StartFadeOut();
		return;
	}
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
