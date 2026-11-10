#include "GameSceneBattleState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "GameContext.h"
#include "DxPlus/DxPlus.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "SoundManager.h"
#include "StatusUI.h"
#include "Collision.h"
#include "CameraShakeType.h"

#include <algorithm>
// ============================================================
// Constructor
// ============================================================

GameSceneBattleState::GameSceneBattleState(
    GameStateMachine& gameStateMachine,
    GameScene& gameScene)
    : GameBaseState(gameStateMachine, gameScene)
{
}

// ============================================================
// Enter
// ============================================================

void GameSceneBattleState::Enter()
{
    SoundManager::StopBGM(RM().GetMusic(ResourceKeys::Music_Title));
    SoundManager::StopBGM(RM().GetMusic(ResourceKeys::Music_Battle));
    SoundManager::StopSE(RM().GetSound(ResourceKeys::Sound_BossEntrance));

    auto* gameContext = gameScene_.GetGameContext();
    auto* player = gameContext->GetPlayer();
    auto& boss = gameContext->GetBoss();

    gameScene_.SaveBattleCheckpoint();
    gameContext->ClearUltimateProjectiles();

    wasTouchingBossBody_ = false;
    wasTailWhipActive_ = false;

    battleIntroWaitRemaining_ = BattleIntroWaitDuration;
    battleIntroFinalWaitElapsed_ = 0.0f;
	// Startの時の音声再生までの待機時間を設定
    bossEntranceSoundRemaining_ = (std::max)(0.0f, BossEntranceSoundDelay);
    bossEntranceSoundPlayed_ = false;

    battleIntroTurnFinished_ = false;
    battleStarted_ = false;

    gameScene_.GetTutorialChecklist().SetVisible(false);
    player->GetStateMachine().SetEventCallback({});

    // 開幕演出中はプレイヤー操作禁止
    player->SetDefMove(false);
    player->SetControlLocked(true);
    player->StopMove();

    // バトル開始演出
    gameScene_.SetBattleEventActive(true);
    gameScene_.StartBattleEventBarsIn();

    // バトル用カメラへ切り替え
    gameScene_.GetCameraDirector().SwitchCamera(
        gameScene_.GetTargetCamera(),
        0.8f
    );

    gameScene_.GetTargetCamera().SetFieldOfView(BattleIntroStartFov);

    // 開幕演出中はボスAIを停止
    boss.SetAIActive(false);

    gameScene_.SetDrawPlayerShadow(true);

    // UI
    {

        StatusUI::InitBoss();
        StatusUI::InitPlayer(player->GetCharData().name);
    
        auto* toLeap = UIM().GetUI(L"toLeapUI");

        if (!toLeap)
        {
            toLeap = UIM().CreateImageUI(
                L"toLeapUI",
                ResourceKeys::TutorialToLeap,
                { 0.0f, 300.0f },
                { 1.0f, 0.0f },
                0.4f
            );
        }

        if (toLeap)
        {
            toLeap->SetCenter({ 0.0f, 0.5f });
            toLeap->SetMoveDistance(100.0f);
            toLeap->SetShowEasing(Motion::Easing::EaseOut);
            toLeap->SetHideEasing(Motion::Easing::EaseIn);

            // 初期状態は即時非表示。
            toLeap->SetFalse();
        }
    }
}

// ============================================================
// Tick
// ============================================================

void GameSceneBattleState::Tick(float deltaTime)
{
    auto* gameContext = gameScene_.GetGameContext();

    auto* player = gameContext->GetPlayer();
    auto& boss = gameContext->GetBoss();
    auto& stage = gameContext->GetStage();

    auto& cameraDirector = gameScene_.GetCameraDirector();
    auto& targetCamera = gameScene_.GetTargetCamera();

    // ========================================================
    // 開幕演出
    // ========================================================

    if (!battleStarted_)
    {
        if (!bossEntranceSoundPlayed_)
        {
			// 開幕演出の音声再生までの待機時間を減算
            bossEntranceSoundRemaining_ = (std::max)(
                0.0f, bossEntranceSoundRemaining_ - (std::max)(0.0f, deltaTime));

            if (bossEntranceSoundRemaining_ <= 0.0f)
            {
                bossEntranceSoundPlayed_ = true;
            }
        }

        stage.Update(deltaTime);

        // 操作ロック中もプレイヤーの物理更新は行う
        player->Update(
            deltaTime,
            stage.GetCollision(),
            stage.GetPosition()
        );

        player->StopMove();

        boss.Update(deltaTime);

        // ボスの注視位置
        Vec3 cameraTarget = boss.GetWorldPosition();

        if (battleIntroTurnFinished_)
        {
            cameraTarget.y += 300.0f;
        }

        targetCamera.Update(
            player->GetWorldPosition(),
            cameraTarget,
            deltaTime
        );

        // ----------------------------------------------------
        // 最初の1.5秒待機
        // ----------------------------------------------------

        if (battleIntroWaitRemaining_ > 0.0f)
        {
            battleIntroWaitRemaining_ = (std::max)(
                0.0f,
                battleIntroWaitRemaining_ - deltaTime
                );
        }
        // ----------------------------------------------------
        // ボスがプレイヤーの方向を向く
        // ----------------------------------------------------

        else if (!battleIntroTurnFinished_)
        {
            battleIntroTurnFinished_ = boss.TurnToTarget(
                player->GetWorldPosition(),
                deltaTime
            );
        }
        // ----------------------------------------------------
        // 回転終了後のカメラ演出
        // ----------------------------------------------------

        else
        {
            battleIntroFinalWaitElapsed_ += deltaTime;

            const float t = (std::min)(
                battleIntroFinalWaitElapsed_ / BattleIntroFinalWaitDuration,
                1.0f
                );

            const float fov =
                BattleIntroStartFov +
                (BattleIntroEndFov - BattleIntroStartFov) * t;

            targetCamera.SetFieldOfView(fov);

            // ------------------------------------------------
            // 開幕演出終了
            // ------------------------------------------------

            if (battleIntroFinalWaitElapsed_ >= BattleIntroFinalWaitDuration)
            {
                StartBattle();
            }
        }

        cameraDirector.Update(deltaTime);

        return;
    }

    // ========================================================
    // 通常バトル
    // ========================================================

    // --------------------------------------------------------
    // Stage更新
    // --------------------------------------------------------

    stage.Update(deltaTime);

    // ========================================================
    // ステータスUI同期
    // ========================================================


    StatusUI::SyncStatusBar(
        L"PlayerUltGauge",
        player->GetUltimateGauge() / Player::UltimateGaugeMax
    );

    StatusUI::SyncStatusBar(
        L"BossHealthGauge",
        1.0f - boss.GetHealthProgress()
    );

    StatusUI::SyncStatusBar(
        L"BossDownGauge",
        1.0f - boss.GetDownGaugeProgress()
    );

    DxPlus::Debug::SetFormatString(
        L"Player Health: %d / %d"
        L"Player Ult Gauge: %.2f / %.2f"
        L"Boss Health: %d / %d"
        L"Boss Down Gauge: %.2f / %.2f",
        player->GetHealth(),
        Player::MaxHealth,
        player->GetUltimateGauge(),
        Player::UltimateGaugeMax,
        boss.GetHealth(),
        Boss::MaxHealth,
        boss.GetDownGaugeProgress() * Const::BOSS_DOWN_GAUGE_MAX,
        Const::BOSS_DOWN_GAUGE_MAX
	);

    // --------------------------------------------------------
    // ゲージ・飛び道具更新
    // --------------------------------------------------------

    if (!player->IsDead() && boss.GetHealth() > 0)
    {
        player->AddUltimateGauge(
            Const::PLAYER_ULTIMATE_GAUGE_PER_SECOND * deltaTime
        );
    }

    gameContext->UpdateUltimateProjectiles(deltaTime);

    // --------------------------------------------------------
    // Player更新
    // --------------------------------------------------------

    const Vec3 stageCenterPosition = stage.GetPosition();

    player->Update(
        deltaTime,
        stage.GetCollision(),
        stageCenterPosition
    );


    // 2階にいる間だけジャンプ案内UIを表示する。
    if (auto* toLeap = UIM().GetUI(L"toLeapUI"))
    {
        const bool shouldShow = player->WasLastOnSecondFloor();

        if (shouldShow)
        {
            // 無効状態または非表示状態から表示モーションを開始する。
            if (toLeap->GetCurrentState() == UIBase::UIState::False)
            {
                toLeap->Show();
            }
        }
        else
        {
            const auto state = toLeap->GetCurrentState();

            // 表示中または表示モーション中なら、
            // 非表示モーションを開始する。
            if (state == UIBase::UIState::Normal ||
                state == UIBase::UIState::Do)
            {
                toLeap->Hide();
            }
        }
    }

    // ========================================================
    // ボス本体へのジャンプ接触判定
    // ========================================================

    const auto state = player->GetCurrentState();

    const bool doubleJump =
        state == PlayerStateType::DoubleJump;

    const bool jumping =
        !player->IsGrounded() &&
        (state == PlayerStateType::Jump || doubleJump);

    const bool touchingBody =
        jumping &&
        stage.GetCollision().IsTouchingCapsule(
            player->GetHitSphere()
        );

    const bool enteredBossBody =
        touchingBody && !wasTouchingBossBody_;

    const bool jumpedFromUpperFloor =
        player->GetJumpStartFloor() == Player::GroundType::UpFloor;

    const bool shouldDamageDownGauge =
        enteredBossBody && jumpedFromUpperFloor;

    if (shouldDamageDownGauge)
    {
        const float damage = doubleJump
            ? Const::BOSS_UPPER_DOUBLE_JUMP_DOWN_DAMAGE
            : Const::BOSS_JUMP_DOWN_DAMAGE;

        boss.ApplyDownGaugeDamage(damage);
    }

    // 二段ジャンプで接触した場合は安全な位置へ戻す
    if (touchingBody && doubleJump)
    {
        player->ReturnToLastSafePosition(stage.GetCollision());
    }

    wasTouchingBossBody_ = touchingBody;

    // ========================================================
    // Boss更新
    // ========================================================

    boss.UpdateClapTargetPosition(player->GetWorldPosition());

    boss.Update(deltaTime);

    if (boss.IsAIActive())
    {
        boss.TurnToTarget(
            player->GetWorldPosition(),
            deltaTime
        );
    }

    // ========================================================
    // Boss攻撃判定
    // ========================================================

    const bool slamHitPlayer = boss.UpdateSlamAttack(
        player->GetWorldPosition(),
        stage.GetWorldPosition()
    );

    const bool handHitPlayer = boss.UpdateHandAttack(
        deltaTime,
        player->GetWorldPosition(),
        stage.GetWorldPosition()
    );

    // ========================================================
    // TailWhip判定
    // ========================================================

    const bool isTailWhipActive = boss.IsTailWhipHitActive();

    // しっぽ攻撃が始まった瞬間だけギミックを発生させる
    if (isTailWhipActive && !wasTailWhipActive_)
    {
        stage.StartShake();

        // 2階にいるプレイヤーを吹き飛ばす
        Vec3 outward =
            player->GetWorldPosition() - stage.GetWorldPosition();

        outward.y = 0.0f;

        if (player->WasLastOnSecondFloor() 
			|| player->GetCurrentState() == PlayerStateType::Climbing)// 柱か二階なら吹き飛ばす
        {
            player->ApplyKnockback(outward, 5000.0f, 0.3f);
        }
    }

    wasTailWhipActive_ = isTailWhipActive;

    Collision::Capsule tailWhipCapsule{};

    if (boss.IsTailWhipActive() && isTailWhipActive)
    {
        tailWhipCapsule = boss.GetTailWhipHitCapsule(
            stage.GetWorldPosition(),
            stage.GetArenaRadius(),
            stage.GetArenaSurfaceY() + 30.0f
        );
    }
    gameContext->UpdateTailWhipSound(tailWhipCapsule);

    const Collision::Sphere playerBody = player->GetHitSphere();

    const bool tailWhipPlayerHit =
        boss.IsTailWhipHitActive() &&
        Collision::IsHitCapsuleSphere(
            tailWhipCapsule,
            playerBody
        );

    if (tailWhipPlayerHit)
    {
        player->TakeDamage(Const::BOSS_TAIL_WHIP_DAMAGE);

        Vec3 outward =
            player->GetWorldPosition() - stage.GetWorldPosition();

        outward.y = 0.0f;

        player->ApplyKnockback(outward, 5000.0f, 0.3f);
    }

    // ========================================================
    // Playerの攻撃判定
    // ========================================================

    auto& playerStateMachine = player->GetStateMachine();

    if (playerStateMachine.IsAttackBallActive())
    {
        const auto& attackBall =
            playerStateMachine.GetAttackBallSphere();

        if (boss.IsHitByAttackSphere(
            attackBall,
            tailWhipCapsule,
            stage.GetBossDownHurtSphere()))
        {
            // 同じ攻撃球による連続ダメージを防止
            if (playerStateMachine.TryRegisterAttackBallHit())
            {
                boss.TakeDamage(Const::PLAYER_NORMAL_ATTACK_DAMAGE);
            }
        }
    }

    // ========================================================
    // Boss攻撃によるPlayerダメージ
    // ========================================================

    if (handHitPlayer)
    {
        player->TakeDamage(Const::BOSS_CLAP_DAMAGE);
    }
    else if (slamHitPlayer)
    {
        player->TakeDamage(Const::BOSS_SLAM_DAMAGE);
    }

    // ========================================================
    // カメラシェイク
    // ========================================================

    auto applyShake = [&cameraDirector](auto& actor)
        {
            const auto type = actor.GetCameraShakeRequest();

            if (type == CameraShakeType::None)
                return;

            const float duration = actor.GetCameraShakeDuration();
            const float strength = actor.GetCameraShakeStrength();
            const float frequency = actor.GetCameraShakeFrequency();

            switch (type)
            {
            case CameraShakeType::Horizontal:
                cameraDirector.StartHorizontalShake(
                    duration,
                    strength,
                    frequency
                );
                break;

            case CameraShakeType::Vertical:
                cameraDirector.StartVerticalShake(
                    duration,
                    strength,
                    frequency
                );
                break;

            case CameraShakeType::Mixed:
                cameraDirector.StartMixedShake(
                    duration,
                    strength,
                    frequency
                );
                break;
            }

            actor.ResetCameraShakeRequest();
        };

    applyShake(boss);
    applyShake(*player);

    // ========================================================
    // カメラ更新・切り替え
    // ========================================================

    // ボスダウン時
    if (boss.IsDowned())
    {
        auto& downCamera = gameScene_.GetBossDownCamera();

        downCamera.Update(
            player->GetWorldPosition(),
            boss.GetWorldPosition(),
            deltaTime
        );

        if (cameraDirector.GetActiveCamera() != &downCamera)
        {
            downCamera.Initialize(cameraDirector.GetCurrentPose());

            cameraDirector.SwitchCamera(
                downCamera,
                1.2f
            );
        }
    }
    // 通常時
    else
    {
        targetCamera.Update(
            player->GetWorldPosition(),
            boss.GetWorldPosition(),
            deltaTime
        );

        // 通常時のFOVに戻す
        targetCamera.SetFieldOfView(60.0f);

        // ダウン・死亡カメラから通常カメラへ戻す
        if (cameraDirector.GetActiveCamera() != &targetCamera)
        {
            cameraDirector.SwitchCamera(
                targetCamera,
                1.2f
            );
        }
    }

    cameraDirector.Update(deltaTime);

    // ========================================================
    // カメラ透過
    // ========================================================

    Collision::ResetTransparentModels();

    Collision::SetModelsBetweenTransparent(
        gameScene_.GetCameraDirector().GetActiveCamera()->GetPosition(),
        player->GetWorldPosition() + Vec3::Up() * 30.0f,
        stage.GetCollision(),
        0.7f
    );

    Collision::SetModelsBetweenTransparent(
        gameScene_.GetCameraDirector().GetActiveCamera()->GetPosition(),
        boss.GetWorldPosition() + Vec3::Up() * 600.0f,
        stage.GetCollision(),
        0.7f
    );

    // ========================================================
    // エフェクト
    // ========================================================

    using namespace DxPlus::Input;

    if (GetButtonDown(PLAYER1) & BUTTON_L3)
    {
        gameContext->GetEffectManager().PlayHit(
            RM().GetEffect(ResourceKeys::Effect_Test),
            player->GetPosition() + Vec3::Up() * 100.0f,
            player->GetYaw() + DX_PI_F
        );
    }

    gameContext->GetEffectManager().Update(deltaTime);
}

// ============================================================
// Exit
// ============================================================

void GameSceneBattleState::StartBattle()
{
    auto* gameContext = gameScene_.GetGameContext();
    auto* player = gameContext->GetPlayer();

    battleStarted_ = true;
    
    SoundManager::PlayBGM(RM().GetMusic(ResourceKeys::Music_Battle));
	// 開幕演出終了
    gameScene_.StartBattleEventBarsOut();
    
	player->SetDefMove(true);                   // プレイヤー操作を許可
	player->SetControlLocked(false);            
    
	gameContext->GetBoss().SetAIActive(true);   //ボスAIを有効化
	StatusUI::Open(player->GetHealth());   //ステータスUIを表示
}

void GameSceneBattleState::SkipIntro()
{
    if (battleStarted_) return;

    auto* gameContext = gameScene_.GetGameContext();
    auto& boss = gameContext->GetBoss();
    gameContext->FaceBossTowardPlayer();
    const Vec3 playerPosition = gameContext->GetPlayer()->GetWorldPosition();

    SoundManager::StopSE(RM().GetSound(ResourceKeys::Sound_BossEntrance));
    bossEntranceSoundPlayed_ = true;
    battleIntroTurnFinished_ = true;

    auto& targetCamera = gameScene_.GetTargetCamera();
	// スキップ時はFOVを通常値に戻す
    targetCamera.SetFieldOfView(60.0f);
    targetCamera.Update(playerPosition, boss.GetWorldPosition(), 1.0f);   
    gameScene_.GetCameraDirector().SwitchCamera(targetCamera, 0.0f);
    gameScene_.SetBattleEventActive(false);
    
    StartBattle();
}

void GameSceneBattleState::Exit()
{
    SoundManager::StopSE(RM().GetSound(ResourceKeys::Sound_BossEntrance));

    auto* gameContext = gameScene_.GetGameContext();
    auto* player = gameContext->GetPlayer();

    gameContext->StopTailWhipSound();
    gameContext->ClearUltimateProjectiles();

    // 操作ロックが残らないようにする
    player->SetDefMove(true);
    player->SetControlLocked(false);

    gameScene_.SetBattleEventActive(false);
    gameScene_.StartBattleEventBarsOut();

    gameScene_.GetTutorialChecklist().SetVisible(false);
    gameScene_.SetDrawPlayerShadow(false);

    StatusUI::Remove();

    // BattleStateで生成したUIを削除する。
    UIM().RemoveUI(L"toLeapUI");
}
