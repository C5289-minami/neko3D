#include "GameSceneBattleState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "GameContext.h"
#include "DxPlus/Debug.h"
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
    DxPlus::Debug::SetFormatString(
        L"GameSceneBattleState::Enter"
    );

    auto* player = gameScene_.GetGameContext()->GetPlayer();

    // バトル用カメラへ切り替える
    gameScene_.GetCameraDirector().SwitchCamera(
        gameScene_.GetTargetCamera(),
        1.2f
    );

    player->SetDefMove(false);
    player->StopMove();

	//StatusUI::InitPlayer();
	//StatusUI::InitBoss();
}

void GameSceneBattleState::Tick(float deltaTime)
{
    DxPlus::Debug::SetFormatString(
        L"GameSceneBattleState::Tick"
    );

    auto* gameContext = gameScene_.GetGameContext();

    auto* player = gameContext->GetPlayer();
    auto& boss = gameContext->GetBoss();

    // GameTestSceneのplayerCamera_.Update()に相当する処理
    gameScene_.GetTargetCamera().Update(
        player->GetPosition(),
        boss.GetPosition(),
        deltaTime
    );

    // カメラの切り替え・遷移を更新
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

    // gauge更新
	StatusUI::SyncStatusBar(L"PlayerHealthGauge", static_cast<float>(player->GetHealth()) / Player::MaxHealth);
	//StatusUI::SyncStatusBar(L"PlayerUltGauge", player->GetUltProgress());
	//StatusUI::SyncStatusBar(L"BossHealthGauge", boss.GetHealthProgress());
	//StatusUI::SyncStatusBar(L"BossDownGauge", boss.GetDownGaugeProgress());
	StatusUI::SyncStatusBar(L"PlayerUltGauge", static_cast<float>(player->GetHealth()) / Player::MaxHealth);
	StatusUI::SyncStatusBar(L"BossHealthGauge", static_cast<float>(player->GetHealth()) / Player::MaxHealth);
	StatusUI::SyncStatusBar(L"BossDownGauge", static_cast<float>(player->GetHealth()) / Player::MaxHealth);
}

void GameSceneBattleState::Exit()
{
    DxPlus::Debug::SetFormatString(
        L"GameSceneBattleState::Exit"
    );
}