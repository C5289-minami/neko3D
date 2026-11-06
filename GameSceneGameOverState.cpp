#include "GameSceneGameOverState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "DxPlus/Debug.h"
#include "UIManager.h"

GameSceneGameOverState::GameSceneGameOverState(GameStateMachine& gameStateMachine, GameScene& gameScene)
	:GameBaseState(gameStateMachine, gameScene)
{
}

void GameSceneGameOverState::Enter()
{
	DxPlus::Debug::SetFormatString(L"GameSceneGameOverState::Enter");
	DxLib::SetMouseDispFlag(TRUE);

	// ゲームオーバーUIを再表示
	{
		if (auto* gameOver = UIM().GetUI(L"Result.GameOver"))
			gameOver->SetTrue();

		if (!UIM().GetUI(L"Result.GameOver"))
		{
			auto* gameOver = UIM().CreateImageUI(
				L"Result.GameOver",
				ResourceKeys::GameOverResult,
				{ DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.5f },
				{},
				2.0f
			);
		}
	}
	// Button
	{
		//まだ表示しないので作成だけ
		if (!UIM().GetUI(L"Result.GameOverRetryButton"))
		{
			auto* retryButton = UIM().CreateButtonUI(
				L"Result.GameOverRetryButton",
				ResourceKeys::Button_Purple.back,
				{ DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.75f },
				[this]()
				{
					if (gameScene_.RestoreBattleCheckpoint())
						stateMachine_.ChangeState(GameStateType::Battle);
				},
				{},
				1.5f
			);
			retryButton->SetButtonHoverSprite(ResourceKeys::Button_Purple.hover);
			retryButton->SetTextFont(RM().GetFont(ResourceKeys::Font_Game));
			retryButton->SetTextScale(1.5f);
			retryButton->SetText(L"リトライ");

			retryButton->SetFalse();
		}

		if (!UIM().GetUI(L"Result.GameOverToTitleButton"))
		{
			auto* toTitleButton = UIM().CreateButtonUI(
				L"Result.GameOverToTitleButton",
				ResourceKeys::Button_Purple.back,
				{ DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.9f },
				[this]()
				{
					gameScene_.ResetForTitle();
					stateMachine_.ChangeState(GameStateType::Title);
				},
				{},
				1.5f
			);
			toTitleButton->SetButtonHoverSprite(ResourceKeys::Button_Purple.hover);
			toTitleButton->SetTextFont(RM().GetFont(ResourceKeys::Font_Game));
			toTitleButton->SetTextScale(1.5f);
			toTitleButton->SetText(L"タイトル");

			toTitleButton->SetFalse();
		}
	}
	selectedIndex_ = 0;
}

void GameSceneGameOverState::Tick(float deltaTime)
{
	DxPlus::Debug::SetFormatString(L"GameSceneGameOverState::Tick");
	gameScene_.UpdateStateCamera(deltaTime, "");

	auto* gameOver = UIM().GetUI(L"Result.GameOver");

	if (!gameOver || gameOver->GetCurrentState() != UIBase::UIState::Normal)
		return;

	auto* retry = UIM().GetButtonUI(L"Result.GameOverRetryButton");
	auto* toTitle = UIM().GetButtonUI(L"Result.GameOverToTitleButton");

	if (!retry || !toTitle)
		return;

	if (!retry->IsVisible())
		retry->SetTrue();

	if (!toTitle->IsVisible())
		toTitle->SetTrue();

	// ボタンの表示アニメーション完了まで待機
	if (retry->GetCurrentState() != UIBase::UIState::Normal ||
		toTitle->GetCurrentState() != UIBase::UIState::Normal)
		return;

	using namespace DxPlus::Input;

	if (GetButtonDown(PLAYER1) & BUTTON_UP)
		selectedIndex_ = 0;

	if (GetButtonDown(PLAYER1) & BUTTON_DOWN)
		selectedIndex_ = 1;

	retry->SetSelected(selectedIndex_ == 0);
	toTitle->SetSelected(selectedIndex_ == 1);

	if (GetButtonDown(PLAYER1) & BUTTON_START)
	{
		if (selectedIndex_ == 0)
			retry->Trigger();
		else
			toTitle->Trigger();
	}
}
void GameSceneGameOverState::Exit()
{
	DxPlus::Debug::SetFormatString(L"GameSceneGameOverState::Enter");
	if (auto* gameOver = UIM().GetUI(L"Result.GameOver"))
		gameOver->Hide();;
	if (auto* retry = UIM().GetButtonUI(L"Result.GameOverRetryButton"))
		retry->Hide();
	if ( auto* toTitle = UIM().GetButtonUI(L"Result.GameOverToTitleButton"))
		toTitle->Hide();
}

