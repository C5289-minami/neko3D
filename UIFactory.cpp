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
	// Test
	{
		const std::wstring key = ResourceKeys::SpriteStudio_TitleCharacter;
		auto* player = RM().GetSpriteStudioPlayer(key);
		if (!player) return;

		const DxPlus::Vec2 testPos(DxPlus::CLIENT_WIDTH * 0.9f, DxPlus::CLIENT_HEIGHT * 0.68f);
		const DxPlus::Vec2 fromBelow(0.8f, 0.2f);// •ûŒü
		const float motionDuration = 1.0f; // o‚Ä‚­‚é‘ŠÔ‚ğw’è
		auto testUI = std::make_unique<SpriteStudioUI>(key, player, testPos, fromBelow, motionDuration);
		testUI->SetShowEasing(Motion::Easing::EaseOut);
		testUI->SetMoveDistance(1000.0f);
		testUI->SetScale(-1.0f); // ¶‰E”½“]
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
