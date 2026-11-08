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
    const int titleBgm = RM().GetMusic(ResourceKeys::Music_Title);
    if (titleBgm >= 0 && DxLib::CheckSoundMem(titleBgm) == 0)
    {
        SoundManager::PlayBGM(titleBgm);
    }

    auto* gameContext = gameScene_.GetGameContext();
    auto* player = gameContext->GetPlayer();
    auto& boss = gameContext->GetBoss();

    gameScene_.SaveBattleCheckpoint();
    gameContext->ClearUltimateProjectiles();

    wasTouchingBossBody_ = false;
    wasTailWhipActive_ = false;

    battleIntroWaitRemaining_ = BattleIntroWaitDuration;
    battleIntroFinalWaitElapsed_ = 0.0f;
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
            toLeap = UIM().CreateImageUI(
                L"toLeapUI",
                ResourceKeys::TutorialToLeap,
                {},
                {1,0});
        if (toLeap)
        {
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
        stage.Update(deltaTime);

        // 操作ロック中もプレイヤーの物理更新は行う
        player->Update(
            deltaTime,
            stage.GetCollision(),
            stage.GetPosition()
        );

        player->StopMove();
        player->CheckFloor();

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
                battleStarted_ = true;
                SoundManager::StopBGM(RM().GetMusic(ResourceKeys::Music_Title));
                SoundManager::PlayBGM(RM().GetMusic(ResourceKeys::Music_Battle));

                gameScene_.StartBattleEventBarsOut();

                // プレイヤー操作を解放
                player->SetDefMove(true);
                player->SetControlLocked(false);

                // ボスAIを開始
                boss.SetAIActive(true);

                StatusUI::Open();
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

    StatusUI::SyncPlayerHealth(player->GetHealth());


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

    player->CheckFloor();

    // 2階にいる間だけジャンプ案内UIを表示する。
    if (auto* toLeap = UIM().GetUI(L"toLeapUI"))
    {
        const bool shouldShow = player->IsSecondFloor();

        // SetTrue()を毎フレーム呼ぶと表示モーションが
        // 最初からやり直されるため、状態が変わったときだけ切り替える。
        if (shouldShow != toLeap->IsVisible())
        {
            if (shouldShow)
            {
                toLeap->SetTrue();
            }
            else
            {
                toLeap->SetFalse();
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

    boss.TurnToTarget(
        player->GetWorldPosition(),
        deltaTime
    );

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

        if (player->IsSecondFloor())
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
        targetCamera.GetPosition(),
        player->GetWorldPosition() + Vec3::Up() * 30.0f,
        stage.GetCollision(),
        0.7f
    );

    Collision::SetModelsBetweenTransparent(
        targetCamera.GetPosition(),
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

void GameSceneBattleState::Exit()
{
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