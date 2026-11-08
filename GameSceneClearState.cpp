#include "GameSceneClearState.h"
#include "GameScene.h"
#include "GameContext.h"
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

	auto& boss = gameScene_.GetGameContext()->GetBoss();
	if (boss.IsDead())
	{
		boss.SetAIActive(false);
		auto& director = gameScene_.GetCameraDirector();
		auto& dieCamera = gameScene_.GetBossDieCamera();
		dieCamera.Initialize(director.GetCurrentPose());
		dieCamera.SetTarget(boss.GetWorldPosition());
		director.SwitchCamera(dieCamera, 1.2f);
	}
	// クリアUI
	{
		auto* clear = UIM().GetUI(L"Result.Clear");

		if (!clear)
		{
			clear = UIM().CreateImageUI(
				L"Result.Clear",
				ResourceKeys::ClearResult,
				{
					DxPlus::CLIENT_WIDTH * 0.5f,
					DxPlus::CLIENT_HEIGHT * 0.5f
				},
				{},
				2.0f
			);
		}

		if (clear)
		{
			clear->SetFalse();
			clear->Show();
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
				[this]()
				{
					if (gameScene_.RestoreBattleCheckpoint())
						stateMachine_.ChangeState(GameStateType::Battle);
				},
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
				[this]()
				{
					gameScene_.ResetForTitle();
					stateMachine_.ChangeState(GameStateType::Title);
				},
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
	selectedIndex_ = 0;
}

void GameSceneClearState::Tick(float deltaTime)
{
	DxPlus::Debug::SetFormatString(L"GameSceneClearState::Tick");

	auto& boss = gameScene_.GetGameContext()->GetBoss();
	if (boss.IsDead() && boss.IsActive())
	{
		boss.Update(deltaTime);
		gameScene_.GetBossDieCamera().SetTarget(boss.GetWorldPosition());
	}
	gameScene_.UpdateStateCamera(deltaTime, "");

	auto* clear = UIM().GetUI(L"Result.Clear");

	if (!clear || clear->GetCurrentState() != UIBase::UIState::Normal)
		return;

	auto* retry = UIM().GetButtonUI(L"Result.ClearRetryButton");
	auto* toTitle = UIM().GetButtonUI(L"Result.ClearToTitleButton");

	if (!retry || !toTitle)
		return;

	if (retry->GetCurrentState() == UIBase::UIState::False)
	{
		retry->Show();
	}

	if (toTitle->GetCurrentState() == UIBase::UIState::False)
	{
		toTitle->Show();
	}

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

void GameSceneClearState::Exit()
{
	DxPlus::Debug::SetFormatString(L"GameSceneClearState::Exit");
	if (auto* clear = UIM().GetUI(L"Result.Clear"))
		clear->Hide();

	if (auto* retry = UIM().GetButtonUI(L"Result.ClearRetryButton"))
		retry->Hide();

	if (auto* toTitle = UIM().GetButtonUI(L"Result.ClearToTitleButton"))
		toTitle->Hide();
}
