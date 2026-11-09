#include "GameSceneClearState.h"
#include "GameScene.h"
#include "GameContext.h"
#include "GameStateMachine.h"
#include "DxPlus/Debug.h"
#include "UIManager.h"

namespace
{
	constexpr const wchar_t*
		ClearHandAnimatorId =
		L"Result.Clear.Hand";

	constexpr float ClearHandXRate = 0.36f;
	constexpr float ClearRetryHandYRate = 0.82f;
	constexpr float ClearTitleHandYRate = 0.92f;

	DxPlus::Vec2 GetClearHandPosition(
		int selectedIndex)
	{
		return {
			DxPlus::CLIENT_WIDTH *
				ClearHandXRate,

			DxPlus::CLIENT_HEIGHT *
				(selectedIndex == 0
					? ClearRetryHandYRate
					: ClearTitleHandYRate)
		};
	}
}

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
				{ DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.8f },
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


	handPrepared_ = false;
	handAnimationStarted_ = false;

	if (auto* context =
		gameScene_.GetGameContext())
	{
		context->HandUIInit();
	}
}

void GameSceneClearState::Tick(float deltaTime)
{
    DxPlus::Debug::SetFormatString(
        L"GameSceneClearState::Tick"
    );

    auto* context =
        gameScene_.GetGameContext();

    if (!context)
    {
        return;
    }

    auto& boss =
        context->GetBoss();

    if (boss.IsDead() &&
        boss.IsActive())
    {
        boss.Update(deltaTime);

        gameScene_
            .GetBossDieCamera()
            .SetTarget(
                boss.GetWorldPosition()
            );
    }

    gameScene_.UpdateStateCamera(
        deltaTime,
        ""
    );

    auto* clear =
        UIM().GetUI(L"Result.Clear");

    if (!clear ||
        clear->GetCurrentState() !=
        UIBase::UIState::Normal)
    {
        return;
    }

    auto* retry =
        UIM().GetButtonUI(
            L"Result.ClearRetryButton"
        );

    auto* toTitle =
        UIM().GetButtonUI(
            L"Result.ClearToTitleButton"
        );

    if (!retry || !toTitle)
    {
        return;
    }

    // ========================================================
    // ボタンと手の表示開始
    // ========================================================

    if (!handPrepared_)
    {
        retry->Show();
        toTitle->Show();

        context->StartActivePlayerHandAnimation(
            ClearHandAnimatorId,
            GetClearHandPosition(selectedIndex_)
        );

        auto* animator =
            UIM().GetTwoFrameAnimator(
                ClearHandAnimatorId
            );

        if (animator)
        {
            // 切り替えアニメーションを一時停止する。
            animator->Pause();

            UIBase* normalHand =
                animator->GetFirstUI();

            UIBase* pressedHand =
                animator->GetSecondUI();

            if (normalHand)
            {
                normalHand->SetFalse();

                // ボタンと同じ1.5秒で表示する。
                normalHand->SetMotionDuration(
                    1.5f
                );

                // 移動なし。
                normalHand->SetToMove({
                    0.0f,
                    0.0f
                    });

                normalHand->Show();
            }

            if (pressedHand)
            {
                pressedHand->SetFalse();
            }
        }

        handPrepared_ = true;
        return;
    }

    // ========================================================
    // ボタンと手の表示完了待ち
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
            ClearHandAnimatorId
        );

    if (!animator)
    {
        return;
    }

    UIBase* normalHand =
        animator->GetFirstUI();

    // 手のフェード完了も待つ。
    if (!normalHand ||
        normalHand->GetCurrentState() !=
        UIBase::UIState::Normal)
    {
        return;
    }

    // ========================================================
    // 通常の2枚アニメーション開始
    // ========================================================

    if (!handAnimationStarted_)
    {
        /*
            Startすると通常画像の表示から始まる。

            フェード完了済みなので、見た目を維持したまま
            通常の押下アニメーションへ移行できる。
        */
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
        GetClearHandPosition(
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

void GameSceneClearState::Exit()
{
	DxPlus::Debug::SetFormatString(L"GameSceneClearState::Exit");
	if (auto* clear = UIM().GetUI(L"Result.Clear"))
		clear->Hide();

	if (auto* retry = UIM().GetButtonUI(L"Result.ClearRetryButton"))
		retry->Hide();

	if (auto* toTitle = UIM().GetButtonUI(L"Result.ClearToTitleButton"))
		toTitle->Hide();

	if (auto* context =
		gameScene_.GetGameContext())
	{
		context->StopPlayerHandAnimation(
			ClearHandAnimatorId
		);
	}
}
