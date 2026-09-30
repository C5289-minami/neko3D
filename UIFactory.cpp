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

	//// Logo
	//{
	//	const DxPlus::Vec2 startButtonPos(DxPlus::CLIENT_WIDTH/2, 400);
   //	std::wstring key = ResourceKeys::UI_Logo;
	//	auto StartButton = std::make_unique<UIBase>(key, startButtonPos, toDown, DEFAULT_SPEED);

	//	manager.AddUI(std::move(StartButton));
	//}

	//const float startX = 100.0f; // ボタンの基準となるX座標
	//const float spaceX = 400.0f; // ボタン間の水平スペース
	//const float baseY = 800.0f; // 基準となるY座標

	//// StartButton
	//{
	//	const DxPlus::Vec2 startButtonPos(startX + spaceX, baseY);
    //	std::wstring key = ResourceKeys::UI_StartButton;
	//	auto StartButton = std::make_unique<ButtonUI>(key, startButtonPos, []() {
	//		SM().SceneOut(Scene::Game,true);
	//		UIFactory::isOpenRule = true;
	//		SM().GetGameContext().GetGameTimeManager().StopTime();
	//		},toUp, 1.5f);
	//	StartButton->SetDisplayCondition([]() { return !SM().titleScenePtr->IsCreditShown(); });
	//	manager.AddUI(std::move(StartButton));
	//}

	//// Credit
	//{
	//	const DxPlus::Vec2 creditPos(startX + spaceX * 3.3f, baseY);
   //	std::wstring key = ResourceKeys::UI_CreditButton;
	//	auto CreditButton = std::make_unique<ButtonUI>(key, creditPos, []() {
	//		SM().titleScenePtr->ToggleCredit();
	//		}, toUp,1.5f);
	//	CreditButton->SetDisplayCondition([]() { return !SM().titleScenePtr->IsCreditShown(); });
	//	manager.AddUI(std::move(CreditButton));
	//}	

	//// 名前入力ボタンの作成
	//{
	//	const DxPlus::Vec2 nameInputPos(DxPlus::CLIENT_WIDTH / 2.0f, baseY);
   //	std::wstring key = ResourceKeys::UI_NameInputButton; // 名前入力用ボタンの画像キー

	//	auto nameInputUI = std::make_unique<NameInputUI>(
	//		key,
	//		nameInputPos,
	//		L"Player1", // 初期表示名
	//		[](const std::wstring& newName) {
	//			// 名前入力が完了（確定）した時の処理
	//			SM().SetPlayerName(newName);
	//		},
	//		toUp,
	//		1.0f
	//	);
	//	nameInputUI->SetDisplayCondition([]() { return !SM().titleScenePtr->IsCreditShown(); });
	//	nameInputUI->SetScale(0.5f);
	//	manager.AddUI(std::move(nameInputUI));
	//}

	//// Collection
	//{
	//	const DxPlus::Vec2 collectionPos(startX + spaceX * 3, baseY);
 //	std::wstring key = ResourceKeys::UI_CollectionButton;
	//	auto CollectionButton = std::make_unique<ButtonUI>(key, collectionPos, []() {
	//		// 図鑑処理
	//		SM().GetGameContext().GetCollectionManager().Trigger();
	//		}, toUp, 0.5f);
	//	manager.AddUI(std::move(CollectionButton));
	//}


	//// Credit
	//{
	//	const DxPlus::Vec2 creditPos(DxPlus::CLIENT_WIDTH/2.0f, DxPlus::CLIENT_HEIGHT/2.0f);
 //	std::wstring key = ResourceKeys::UI_Credit;
	//	auto CreditButton = std::make_unique<UIBase>(key, creditPos, toUp, 0.5f);
	//	CreditButton->SetDisplayCondition([]() { return SM().titleScenePtr->IsCreditShown(); });
	//	manager.AddUI(std::move(CreditButton));
	//}
	//// BackToTitle
	//{
	//	const DxPlus::Vec2 backToTitlePos(DxPlus::CLIENT_WIDTH * 0.82f, DxPlus::CLIENT_HEIGHT * 0.84f);
 //	std::wstring key = ResourceKeys::UI_ToBack;
	//	auto BackToTitleButton = std::make_unique<ButtonUI>(key, backToTitlePos, []() {
	//		// タイトルに戻る処理
	//		SM().titleScenePtr->ToggleCredit();
	//		}, toUp, 0.5f);
	//	BackToTitleButton->SetDisplayCondition([]() { return SM().titleScenePtr->IsCreditShown(); });
	//	BackToTitleButton->SetScale(0.8f); // ボタンを小さくする
	//	manager.AddUI(std::move(BackToTitleButton));
	//}


	// Test
	{
		const std::wstring key = ResourceKeys::SpriteStudio_TitleCharacter;
		auto* player = RM().GetSpriteStudioPlayer(key);
		if (!player) return;

		const DxPlus::Vec2 testPos(DxPlus::CLIENT_WIDTH * 0.78f, DxPlus::CLIENT_HEIGHT * 0.68f);
		const DxPlus::Vec2 fromBelow(0.0f, 1.0f);
		auto testUI = std::make_unique<SpriteStudioUI>(key, player, testPos, fromBelow, 1.0f);
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
