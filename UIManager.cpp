#include "UIManager.h"
#include "SceneManager.h"
#include "ResourceKeys.h"
#include "Button.h"
#include "DxPlus/DxPlus.h"
#include "UIFactory.h"


void UIManager::Init()
{
	UIs.clear();
	SceneID type = SM().GetCurrentSceneID();

	if (type == SceneID::Title) {
		UIFactory::CreateTitleUI(*this);
	}
	else if (type == SceneID::Game) {
		UIFactory::CreateGameUI(*this);
	}
	else if (type == SceneID::Result) {
		UIFactory::CreateResultUI(*this);
	}
	


}

void UIManager::Update(float deltaTime)
{
	for (auto& ui : UIs) {
		ui->Update(deltaTime);
	}
	//DxPlus::Debug::SetFormatString(L"Current UI Type: %d", static_cast<int>(currentUIType)); // デバッグ用に現在のUIタイプを表示
}

void UIManager::Draw(int drawZOrder)
{
	for (auto& ui : UIs) {
		if (drawZOrder == -1 || ui->GetZOrder() == drawZOrder) {
			ui->Draw();
		}
	}
}


