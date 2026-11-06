#include "GameSceneBattleState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "GameContext.h"
#include "DxPlus/DxPlus.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "StatusUI.h"
#include "Collision.h"
#include "CameraShakeType.h"
#include "UIManager.h"
#include <algorithm>

GameSceneBattleState::GameSceneBattleState(
    GameStateMachine& gameStateMachine,
    GameScene& gameScene)
    : GameBaseState(gameStateMachine, gameScene)
{
}

void GameSceneBattleState::Enter()
{
    auto* player = gameScene_.GetGameContext()->GetPlayer();
	auto& boss = gameScene_.GetGameContext()->GetBoss();

    wasTouchingBossBody_ = false;
    wasTailWhipActive_ = false;
    battleIntroTurnRemaining_ = BattleIntroTurnDuration;
    battleIntroWaitRemaining_ = BattleIntroWaitDuration;
    battleStarted_ = false;

    // チュートリアルの入力通知とチェックリストを、Battleへ持ち越さない。
    gameScene_.GetTutorialChecklist().SetVisible(false);
    player->GetStateMachine().SetEventCallback({});
    gameScene_.SetBattleEventActive(true);
    gameScene_.StartBattleEventBarsIn();

    // バトル用カメラへ切り替える
    gameScene_.GetCameraDirector().SwitchCamera(
        gameScene_.GetTargetCamera(),
        0.8f
    );

    player->SetDefMove(false);
    player->SetControlLocked(true);
    player->StopMove();

   boss.SetAIActive(false);

    StatusUI::InitBoss();
    gameScene_.SetDrawPlayerShadow(true);
}
void GameSceneBattleState::Tick(float deltaTime)
{
    auto* gameContext = gameScene_.GetGameContext();

    auto* player = gameContext->GetPlayer();
    auto& boss = gameContext->GetBoss();
    auto& stage = gameContext->GetStage();
    // =============================
    // 開幕演出。Playerは入力を止めたまま、重力と床の当たり判定だけ更新する。
    // =============================
    if (!battleStarted_)
    {
        stage.Update(deltaTime);
        player->Update(deltaTime, stage.GetCollision(), stage.GetPosition());
        player->CheckFloor();

        boss.Update(deltaTime);

        // ボスがプレイヤーを向く
        if (battleIntroTurnRemaining_ > 0.0f)
        {
            const bool turnFinished = boss.TurnToTarget(
                player->GetWorldPosition(),
                deltaTime
            );

            if (turnFinished)
            {
                battleIntroTurnRemaining_ = 0.0f;
            }
        }
        else
        {
            battleIntroWaitRemaining_ = (std::max)(
                0.0f,
                battleIntroWaitRemaining_ - deltaTime
                );
        }

        auto& targetCamera = gameScene_.GetTargetCamera();

        targetCamera.Update(
            player->GetWorldPosition(),
            boss.GetWorldPosition(),
            deltaTime
        );

        targetCamera.SetFieldOfView(55.0f);
        gameScene_.GetCameraDirector().Update(deltaTime);

        if (battleIntroTurnRemaining_ <= 0.0f &&
            battleIntroWaitRemaining_ <= 0.0f)
        {
            battleStarted_ = true;

            gameScene_.StartBattleEventBarsOut();

            player->SetDefMove(true);
            player->SetControlLocked(false);

            boss.SetAIActive(true);
        }

        return;
    }

    // =============================
    // Stage更新
    // =============================
    stage.Update(deltaTime);

    // =============================
    // =============================
    // Player更新・床判定
    // =============================
    const Vec3 stageCenterPosition = stage.GetPosition();

    player->Update(
        deltaTime,
        stage.GetCollision(),
        stageCenterPosition
    );

    player->CheckFloor();

    // =============================
    // Boss本体へのジャンプ接触判定
    // =============================
    const auto state = player->GetCurrentState();

    const bool doubleJump = state == PlayerStateType::DoubleJump;

    const bool jumping =
        !player->IsGrounded() &&
        (state == PlayerStateType::Jump || doubleJump);

    const bool touchingBody =
        jumping &&
        stage.GetCollision().IsTouchingCapsule(player->GetHitSphere());

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

    // =============================
    // Boss更新
    // =============================
    boss.UpdateClapTargetPosition(player->GetWorldPosition());

    boss.Update(deltaTime);

    boss.TurnToTarget(
        player->GetWorldPosition(),
        deltaTime
    );

    // =============================
    // Boss攻撃判定
    // =============================
    const bool slamHitPlayer = boss.UpdateSlamAttack(
        player->GetWorldPosition(),
        stage.GetWorldPosition()
    );

    const bool handHitPlayer = boss.UpdateHandAttack(
        deltaTime,
        player->GetWorldPosition(),
        stage.GetWorldPosition()
    );

    // =============================
    // TailWhipギミック
    // =============================
    const bool isTailWhipActive = boss.IsTailWhipHitActive();

    if (isTailWhipActive && !wasTailWhipActive_)
    {
        stage.StartShake();

        Vec3 outward =
            player->GetWorldPosition() - stage.GetWorldPosition();

        outward.y = 0.0f;

        if (player->IsSecondFloor())
        {
            player->ApplyKnockback(outward, 5000.0f, 0.3f);
        }
    }

    wasTailWhipActive_ = isTailWhipActive;

    // =============================
    // TailWhip当たり判定
    // =============================
    Collision::Capsule tailWhipCapsule{};

    if (boss.IsTailWhipActive() && boss.IsTailWhipHitActive())
    {
        tailWhipCapsule = boss.GetTailWhipHitCapsule(
            stage.GetWorldPosition(),
            stage.GetArenaRadius(),
            stage.GetArenaSurfaceY() + 30.0f
        );
    }

    const Collision::Sphere playerBody = player->GetHitSphere();

    const bool tailWhipPlayerHit =
        boss.IsTailWhipHitActive() &&
        Collision::IsHitCapsuleSphere(
            tailWhipCapsule,
            playerBody
        );

    // =============================
    // Playerの攻撃判定
    // =============================
    auto& playerStateMachine = player->GetStateMachine();

    if (playerStateMachine.IsAttackBallActive())
    {
        const auto& attackBall =
            playerStateMachine.GetAttackBallSphere();

        if (boss.IsHitByAttackSphere(attackBall, tailWhipCapsule))
        {
            // 同じ攻撃球による連続ダメージを防止
            if (playerStateMachine.TryRegisterAttackBallHit())
            {
                boss.TakeDamage();
            }
        }
    }

    // =============================
    // Player被ダメージ処理
    // =============================
    if (handHitPlayer || slamHitPlayer)
    {
        player->TakeDamage(boss.GetScale());
    }

    if (tailWhipPlayerHit)
    {
        player->TakeDamage(boss.GetScale());

        Vec3 outward =
            player->GetWorldPosition() - stage.GetWorldPosition();

        outward.y = 0.0f;

        player->ApplyKnockback(outward, 5000.0f, 0.3f);
    }

    // =============================
    // カメラシェイク
    // =============================
    auto applyShake = [this](auto& actor)
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
                gameScene_.GetCameraDirector().StartHorizontalShake(
                    duration, strength, frequency
                );
                break;

            case CameraShakeType::Vertical:
                gameScene_.GetCameraDirector().StartVerticalShake(
                    duration, strength, frequency
                );
                break;

            case CameraShakeType::Mixed:
                gameScene_.GetCameraDirector().StartMixedShake(
                    duration, strength, frequency
                );
                break;
            }

            actor.ResetCameraShakeRequest();
        };

    applyShake(boss);
    applyShake(*player);

    // =============================
    // カメラ更新・切り替え
    // =============================
    auto& cameraDirector = gameScene_.GetCameraDirector();

    // ボス死亡時
    if (boss.IsDead())
    {
        auto& dieCamera = gameScene_.GetBossDieCamera();

        dieCamera.SetTarget(boss.GetWorldPosition());

        if (cameraDirector.GetActiveCamera() != &dieCamera)
        {
            dieCamera.Initialize(cameraDirector.GetCurrentPose());
            cameraDirector.SwitchCamera(dieCamera, 1.2f);
        }
    }
    // ボスダウン時
    else if (boss.IsDowned())
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
            cameraDirector.SwitchCamera(downCamera, 1.2f);
        }
    }
    // 通常時
    else
    {
        auto& targetCamera = gameScene_.GetTargetCamera();

        targetCamera.Update(
            player->GetWorldPosition(),
            boss.GetWorldPosition(),
            deltaTime
        );

        targetCamera.SetFieldOfView(60.0f);

        if (cameraDirector.GetActiveCamera() != &targetCamera)
        {
            cameraDirector.SwitchCamera(targetCamera, 1.2f);
        }
    }

    cameraDirector.Update(deltaTime);
    // =============================
    // カメラ透過
    // =============================
    Collision::ResetTransparentModels();

    Collision::SetModelsBetweenTransparent(
        gameScene_.GetTargetCamera().GetPosition(),
        player->GetWorldPosition() + Vec3::Up() * 30.0f,
        stage.GetCollision(),
        0.7f
    );

    Collision::SetModelsBetweenTransparent(
        gameScene_.GetTargetCamera().GetPosition(),
        boss.GetWorldPosition() + Vec3::Up() * 600.0f,
        stage.GetCollision(),
        0.7f
    );

    // =============================
    // 入力・エフェクト
    // =============================
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

    // =============================
    // ステータスUI更新
    // =============================
    StatusUI::SyncStatusBar(
        L"BossHealthGauge",
        boss.GetHealthProgress()
    );

    StatusUI::SyncStatusBar(
        L"BossDownGauge",
        boss.GetDownGaugeProgress()
    );
}

void GameSceneBattleState::Exit()
{
  gameScene_.SetBattleEventActive(false);
    gameScene_.StartBattleEventBarsOut();
    gameScene_.GetTutorialChecklist().SetVisible(false);
    gameScene_.SetDrawPlayerShadow(false);
    UIM().RemoveUI(L"BossStatusBackground");
    UIM().RemoveUI(L"BossHealthGauge");
    UIM().RemoveUI(L"BossDownGauge");
}