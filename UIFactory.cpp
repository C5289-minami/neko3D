#include "UIFactory.h"
#include "ResourceKeys.h"
#include "ButtonUI.h"
#include "Scene.h"
#include "UIBase.h"
#include <string>

#include "TitleScene.h"
#include "ResourceManager.h"
#include "SpriteStudioUI.h"

void UIFactory::CreateTitleUI(UIManager& manager)
{
	const DxPlus::Vec2 toLeft(-1, 0); // 左方向に出てくるモーション
	const DxPlus::Vec2 toUp(0, -1); // 上方向に出てくるモーション
	const DxPlus::Vec2 toDown(0, 1); // 下方向に出てくるモーション
	const DxPlus::Vec2 toRight(1, 0); // 右方向に出てくるモーション

	constexpr float DEFAULT_SPEED = 1.0f; // 出てくる速度

	
	// Test
	{
		const std::wstring key = ResourceKeys::SpriteStudio_TitleCharacter;
		auto* player = RM().GetSpriteStudioPlayer(key);
		if (!player) return;

		const DxPlus::Vec2 testPos(DxPlus::CLIENT_WIDTH * 0.78f, DxPlus::CLIENT_HEIGHT * 0.68f);
		const DxPlus::Vec2 fromBelow(0.0f, 1.0f);
		auto testUI = std::make_unique<SpriteStudioUI>(key, player, testPos, fromBelow, 3.0f);
		testUI->SetShowEasing(Motion::Easing::EaseInOut);
		testUI->SetMoveDistance(500.0f);
		testUI->SetScale(0.5f);
		manager.AddUI(std::move(testUI));
	}
}



void UIFactory::CreateGameUI(UIManager& manager)
{
	
}

void UIFactory::CreateResultUI(UIManager& manager)
{
	
}
