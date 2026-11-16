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
    else if (DxLib::CheckHitKey(KEY_INPUT_F7))
    {
        SM().RequestPresentationScene(SceneID::CatViewer);
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
    ImGui::Begin("TitleScene Debug Menu");
    ImGui::TextUnformatted("F1: SpriteStudio / F2: Shader / F3: Motion & Room");
    ImGui::TextUnformatted("F4: Status / F5: Model Config / F6: Game");
    ImGui::TextUnformatted("Enter: Battle Test / F7: Cat Animation Viewer");
    ImGui::TextUnformatted("M: Next / Shift+M: Previous / F11: Scene Menu / F10: Hide UI");
    ImGui::End();
}
#endif
