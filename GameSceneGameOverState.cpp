#include "GameSceneGameOverState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "DxPlus/Debug.h"
#include "UIManager.h"
#include "GameContext.h"

namespace
{
	constexpr const wchar_t*
		GameOverHandAnimatorId =
		L"Result.GameOver.Hand";

	constexpr float GameOverHandXRate = 0.36f;
	constexpr float GameOverRetryHandYRate = 0.82f;
	constexpr float GameOverTitleHandYRate = 0.92f;

	DxPlus::Vec2 GetGameOverHandPosition(
		int selectedIndex)
	{
		return {
			DxPlus::CLIENT_WIDTH *
				GameOverHandXRate,

			DxPlus::CLIENT_HEIGHT *
				(selectedIndex == 0
					? GameOverRetryHandYRate
					: GameOverTitleHandYRate)
		};
	}
}

GameSceneGameOverState::GameSceneGameOverState(GameStateMachine& gameStateMachine, GameScene& gameScene)
	:GameBaseState(gameStateMachine, gameScene)
{
}

void GameSceneGameOverState::Enter()
{
	DxPlus::Debug::SetFormatString(L"GameSceneGameOverState::Enter");
	DxLib::SetMouseDispFlag(TRUE);

	// ゲームオーバーUI
	{
		auto* gameOver = UIM().GetUI(L"Result.GameOver");

		if (!gameOver)
		{
			gameOver = UIM().CreateImageUI(
				L"Result.GameOver",
				ResourceKeys::GameOverResult,
				{
					DxPlus::CLIENT_WIDTH * 0.5f,
					DxPlus::CLIENT_HEIGHT * 0.5f
				},
				{},
				2.0f
			);
		}

		if (gameOver)
		{
			gameOver->SetFalse();
			gameOver->Show();
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
				{ DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.8f },
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


	handPrepared_ = false;
	handAnimationStarted_ = false;

	if (auto* context =
		gameScene_.GetGameContext())
	{
		// UIM().Clear()後でも手UIを使えるようにする。
		// この時点ではAnimatorを開始しない。
		context->HandUIInit();
	}
}

void GameSceneGameOverState::Tick(float deltaTime)
{
    DxPlus::Debug::SetFormatString(
        L"GameSceneGameOverState::Tick"
    );

    gameScene_.UpdateStateCamera(
        deltaTime,
        ""
    );

    auto* gameOver =
        UIM().GetUI(L"Result.GameOver");

    // ゲームオーバー画像の表示完了を待つ。
    if (!gameOver ||
        gameOver->GetCurrentState() !=
        UIBase::UIState::Normal)
    {
        return;
    }

    auto* retry =
        UIM().GetButtonUI(
            L"Result.GameOverRetryButton"
        );

    auto* toTitle =
        UIM().GetButtonUI(
            L"Result.GameOverToTitleButton"
        );

    if (!retry || !toTitle)
    {
        return;
    }

    // ========================================================
    // ボタンと通常手の表示開始
    // ========================================================

    if (!handPrepared_)
    {
        retry->Show();
        toTitle->Show();

        auto* context =
            gameScene_.GetGameContext();

        if (context)
        {
            // 使用中Playerの通常手と押下手をAnimatorに設定する。
            context->StartActivePlayerHandAnimation(
                GameOverHandAnimatorId,
                GetGameOverHandPosition(
                    selectedIndex_
                )
            );
        }

        auto* animator =
            UIM().GetTwoFrameAnimator(
                GameOverHandAnimatorId
            );

        if (animator)
        {
            // Animatorによる画像切り替えだけ停止する。
            // 表示中のUI自体は非表示にしない。
            animator->Pause();

            UIBase* normalHand =
                animator->GetFirstUI();

            UIBase* pressedHand =
                animator->GetSecondUI();

            if (normalHand)
            {
                normalHand->SetFalse();

                // ボタンと同じ表示時間にする。
                normalHand->SetMotionDuration(
                    1.5f
                );

                // 位置移動なしで表示する。
                normalHand->SetToMove({
                    0.0f,
                    0.0f
                    });

                normalHand->Show();
            }

            // フェード中は通常手だけ表示する。
            if (pressedHand)
            {
                pressedHand->SetFalse();
            }
        }

        handPrepared_ = true;
        return;
    }

    // ========================================================
    // ボタンと通常手の表示完了待ち
    // ========================================================

    if (retry->GetCurrentState() !=
        UIBase::UIState::Normal ||
        toTitle->GetCurrentState() !=
        UIBase::UIState::Normal)
    {
        return;
    }

    auto* animator =
        UIM().GetTwoFrameAnimator(
            GameOverHandAnimatorId
        );

    if (!animator)
    {
        return;
    }

    UIBase* normalHand =
        animator->GetFirstUI();

    // 通常手の表示完了も待つ。
    if (!normalHand ||
        normalHand->GetCurrentState() !=
        UIBase::UIState::Normal)
    {
        return;
    }

    // ========================================================
    // 2枚切り替えアニメーション開始
    // ========================================================

    if (!handAnimationStarted_)
    {
        animator->Start();
        handAnimationStarted_ = true;
    }

    // ========================================================
    // 選択操作
    // ========================================================

    using namespace DxPlus::Input;

    if (GetButtonDown(PLAYER1) &
        BUTTON_UP)
    {
        selectedIndex_ = 0;
    }

    if (GetButtonDown(PLAYER1) &
        BUTTON_DOWN)
    {
        selectedIndex_ = 1;
    }

    retry->SetSelected(
        selectedIndex_ == 0
    );

    toTitle->SetSelected(
        selectedIndex_ == 1
    );

    animator->SetAnchorPosition(
        GetGameOverHandPosition(
            selectedIndex_
        )
    );

    if (GetButtonDown(PLAYER1) &
        BUTTON_START)
    {
        if (selectedIndex_ == 0)
        {
            retry->Trigger();
        }
        else
        {
            toTitle->Trigger();
        }
    }
}

void GameSceneGameOverState::Exit()
{
	DxPlus::Debug::SetFormatString(
		L"GameSceneGameOverState::Exit"
	);

	if (auto* gameOver = UIM().GetUI(L"Result.GameOver"))
		gameOver->Hide();

	if (auto* retry = UIM().GetButtonUI(L"Result.GameOverRetryButton"))
		retry->Hide();

	if (auto* toTitle = UIM().GetButtonUI(L"Result.GameOverToTitleButton"))
		toTitle->Hide();

	if (auto* context =
		gameScene_.GetGameContext())
	{
		context->StopPlayerHandAnimation(
			GameOverHandAnimatorId
		);
	}
}