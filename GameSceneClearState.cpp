#include "GameSceneClearState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "DxPlus/Debug.h"
#include "UIManager.h"

GameSceneClearState::GameSceneClearState(GameStateMachine& gameStateMachine, GameScene& gameScene)
    :GameBaseState(gameStateMachine, gameScene)
{
}

void GameSceneClearState::Enter()
{
    DxPlus::Debug::SetFormatString(L"GameSceneClearState::Enter");
	DxLib::SetMouseDispFlag(TRUE);

	// クリアUIを再表示
	{
		if (auto* clear = UIM().GetUI(L"Result.Clear"))
			clear->SetTrue();

		if (!UIM().GetUI(L"Result.Clear"))
		{
			UIM().CreateImageUI(
				L"Result.Clear",
				ResourceKeys::ClearResult,
				{ DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.5f },
				{},
				2.0f
			);
		}
	}

	// Button
	{
		// まだ表示しないので作成だけ
		if (!UIM().GetUI(L"Result.ClearRetryButton"))
		{
			auto* retryButton = UIM().CreateButtonUI(
				L"Result.ClearRetryButton",
				ResourceKeys::Button_Rose.back,
				{ DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.75f },
				[]() { PostQuitMessage(0); },
				{},
				1.5f
			);
			retryButton->SetButtonHoverSprite(ResourceKeys::Button_Rose.hover);
			retryButton->SetTextFont(RM().GetFont(ResourceKeys::Font_Game));
			retryButton->SetTextScale(1.5f);
			retryButton->SetText(L"リトライ");

			retryButton->SetFalse();
		}

		if (!UIM().GetUI(L"Result.ClearToTitleButton"))
		{
			auto* toTitleButton = UIM().CreateButtonUI(
				L"Result.ClearToTitleButton",
				ResourceKeys::Button_Rose.back,
				{ DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.9f },
				[]() { PostQuitMessage(0); },
				{},
				1.5f
			);
			toTitleButton->SetButtonHoverSprite(ResourceKeys::Button_Rose.hover);
			toTitleButton->SetTextFont(RM().GetFont(ResourceKeys::Font_Game));
			toTitleButton->SetTextScale(1.5f);
			toTitleButton->SetText(L"タイトル");

			toTitleButton->SetFalse();
		}
	}
}

void GameSceneClearState::Tick(float deltaTime)
{
    DxPlus::Debug::SetFormatString(L"GameSceneClearState::Enter");
    gameScene_.UpdateStateCamera(deltaTime, "");

	auto* clear = UIM().GetUI(L"Result.Clear");

	if (clear && clear->GetCurrentState() == UIBase::UIState::Normal)
	{
		if (auto* retry = UIM().GetUI(L"Result.ClearRetryButton"))
		{
			if (!retry->IsVisible())
				retry->SetTrue();
		}

		if (auto* toTitle = UIM().GetUI(L"Result.ClearToTitleButton"))
		{
			if (!toTitle->IsVisible())
				toTitle->SetTrue();
		}
	}
}

void GameSceneClearState::Exit()
{
    DxPlus::Debug::SetFormatString(L"GameSceneClearState::Enter");
	if (auto* clear = UIM().GetUI(L"Result.Clear"))
		clear->Hide();

	if (auto* retry = UIM().GetButtonUI(L"Result.ClearRetryButton"))
		retry->Hide();

	if (auto* toTitle = UIM().GetButtonUI(L"Result.ClearToTitleButton"))
		toTitle->Hide();
}
