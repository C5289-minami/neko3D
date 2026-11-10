#include "GameSceneClearState.h"
#include "GameScene.h"
#include "GameContext.h"
#include "GameStateMachine.h"
#include "DxPlus/Debug.h"
#include "UIManager.h"
#include "ResourceManager.h"
#include "SoundManager.h"

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
    SoundManager::PlaySE(RM().GetSound(ResourceKeys::Sound_BossEntrance));

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
            // 2枚切り替えだけ一時停止する。
            animator->Pause();

            UIBase* normalHand =
                animator->GetFirstUI();

            UIBase* pressedHand =
                animator->GetSecondUI();

            if (normalHand)
            {
                normalHand->SetFalse();

                // ボタンと同じ時間で表示する。
                normalHand->SetMotionDuration(
                    1.5f
                );

                // 移動なしで表示する。
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
    // ボタンの表示完了待ち
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

    // ========================================================
    // 手のアニメーション開始判定
    // 入力処理とは独立させる
    // ========================================================

    if (animator &&
        !handAnimationStarted_)
    {
        UIBase* normalHand =
            animator->GetFirstUI();

        // 猫の手のフェードが終わったら、
        // 通常の2枚アニメーションを開始する。
        if (normalHand &&
            normalHand->GetCurrentState() ==
            UIBase::UIState::Normal)
        {
            animator->Start();
            handAnimationStarted_ = true;
        }
    }

    // ========================================================
    // 選択操作
    // 手がフェード中でも操作可能
    // ========================================================

    using namespace DxPlus::Input;

    constexpr int SelectionCount = 2;

    const int buttonDown =
        GetButtonDown(PLAYER1);

    bool selectionChanged = false;

    if (buttonDown & BUTTON_UP)
    {
        selectedIndex_ =
            (selectedIndex_ - 1 +
                SelectionCount) %
            SelectionCount;

        selectionChanged = true;
    }

    if (buttonDown & BUTTON_DOWN)
    {
        selectedIndex_ =
            (selectedIndex_ + 1) %
            SelectionCount;

        selectionChanged = true;
    }

    if (selectionChanged)
    {
        SoundManager::PlaySE(
            RM().GetSound(
                ResourceKeys::Sound_Rain
            )
        );
    }

    retry->SetSelected(
        selectedIndex_ == 0
    );

    toTitle->SetSelected(
        selectedIndex_ == 1
    );

    // Animatorが存在する場合のみ、手の位置を更新する。
    if (animator)
    {
        animator->SetAnchorPosition(
            GetClearHandPosition(
                selectedIndex_
            )
        );
    }

    // 手が1枚目、2枚目、フェード中のどれでも決定可能。
    if (buttonDown & BUTTON_START)
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
	SoundManager::StopSE(RM().GetSound(ResourceKeys::Sound_BossEntrance));
	SoundManager::StopSE(RM().GetSound(ResourceKeys::Sound_Rain));

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
