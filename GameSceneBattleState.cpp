#include "GameSceneBattleState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "GameContext.h"
#include "DxPlus/DxPlus.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "StatusUI.h"

GameSceneBattleState::GameSceneBattleState(
    GameStateMachine& gameStateMachine,
    GameScene& gameScene)
    : GameBaseState(gameStateMachine, gameScene)
{
}

void GameSceneBattleState::Enter()
{
    auto* player = gameScene_.GetGameContext()->GetPlayer();

    // バトル用カメラへ切り替える
    gameScene_.GetCameraDirector().SwitchCamera(
        gameScene_.GetTargetCamera(),
        0.8f
    );

    player->SetDefMove(false);
    player->StopMove();
    gameScene_.SetDrawPlayerShadow(true);
}

void GameSceneBattleState::Tick(float deltaTime)
{
    auto* gameContext = gameScene_.GetGameContext();

    auto* player = gameContext->GetPlayer();
    auto& boss = gameContext->GetBoss();
    auto& stage = gameContext->GetStage();
	auto& taskManager = gameScene_.GetTutorialTaskManager();

    const Vec3 stageCenterPosition = stage.GetPosition();

    taskManager.Update(deltaTime);

    if (taskManager.IsAllFinished())
    {
        gameScene_.GetTutorialChecklist().SetVisible(false);
        // 通知先を解除
        player->GetStateMachine().SetEventCallback({});
    }

    // Bossの手攻撃判定
    if (boss.UpdateHandAttack(
        deltaTime,
        player->GetWorldPosition(),
        stage.GetWorldPosition()))
    {
        player->TakeDamage();
    }

    // Playerの攻撃球とBossの手の当たり判定
    auto& playerStateMachine = player->GetStateMachine();

    if (playerStateMachine.IsAttackBallActive())
    {
        const auto& attackBall =
            playerStateMachine.GetAttackBallSphere();

        if (boss.IsHitByAttackSphere(attackBall))
        {
            // 同じ攻撃球による連続ダメージを防止
            if (playerStateMachine.TryRegisterAttackBallHit())
            {
                boss.TakeDamage();
            }
        }
    }

    // カメラ更新
    gameScene_.GetTargetCamera().Update(
        player->GetPosition(),
        boss.GetWorldPosition(),
        deltaTime
    );

    gameScene_.GetCameraDirector().Update(deltaTime);

    // L3でヒットエフェクトを再生
    using namespace DxPlus::Input;

    if (GetButtonDown(PLAYER1) & BUTTON_L3)
    {
        gameContext->GetEffectManager().PlayHit(
            RM().GetEffect(ResourceKeys::Effect_Test),
            player->GetPosition() + Vec3::Up() * 100.0f,
            player->GetYaw() + DX_PI_F
        );
    }

    // エフェクト更新
    gameContext->GetEffectManager().Update(deltaTime);

    //// ステータスUI更新
    //StatusUI::SyncStatusBar(
    //    L"PlayerHealthGauge",
    //    static_cast<float>(player->GetHealth()) / Player::MaxHealth
    //);

    //StatusUI::SyncStatusBar(
    //    L"PlayerUltGauge",
    //    player->GetUltProgress()
    //);

    //StatusUI::SyncStatusBar(
    //    L"BossHealthGauge",
    //    boss.GetHealthProgress()
    //);

    //StatusUI::SyncStatusBar(
    //    L"BossDownGauge",
    //    boss.GetDownGaugeProgress()
    //);
}

void GameSceneBattleState::Exit()
{
    gameScene_.SetDrawPlayerShadow(false);
}