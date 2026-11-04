#include "GameSceneBattleState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "GameContext.h"
#include "DxPlus/DxPlus.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "StatusUI.h"
#include "UIManager.h"

GameSceneBattleState::GameSceneBattleState(
    GameStateMachine& gameStateMachine,
    GameScene& gameScene)
    : GameBaseState(gameStateMachine, gameScene)
{
}

void GameSceneBattleState::Enter()
{
    auto* player = gameScene_.GetGameContext()->GetPlayer();

    // �o�g���p�J�����֐؂�ւ���
    gameScene_.GetCameraDirector().SwitchCamera(
        gameScene_.GetTargetCamera(),
        0.8f
    );

    player->SetDefMove(false);
    player->StopMove();
    StatusUI::InitBoss();
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
        // �ʒm�������
        player->GetStateMachine().SetEventCallback({});
    }

    // Boss�̎�U������
    if (boss.UpdateHandAttack(
        deltaTime,
        player->GetWorldPosition(),
        stage.GetWorldPosition()))
    {
        player->TakeDamage();
    }

    // Player�̍U������Boss�̎�̓����蔻��
    auto& playerStateMachine = player->GetStateMachine();

    if (playerStateMachine.IsAttackBallActive())
    {
        const auto& attackBall =
            playerStateMachine.GetAttackBallSphere();

        if (boss.IsHitByAttackSphere(attackBall))
        {
            // �����U�����ɂ��A���_���[�W��h�~
            if (playerStateMachine.TryRegisterAttackBallHit())
            {
                boss.TakeDamage();
            }
        }
    }

    // �J�����X�V
    gameScene_.GetTargetCamera().Update(
        player->GetPosition(),
        boss.GetWorldPosition(),
        deltaTime
    );

    gameScene_.GetCameraDirector().Update(deltaTime);

    // L3�Ńq�b�g�G�t�F�N�g���Đ�
    using namespace DxPlus::Input;

    if (GetButtonDown(PLAYER1) & BUTTON_L3)
    {
        gameContext->GetEffectManager().PlayHit(
            RM().GetEffect(ResourceKeys::Effect_Test),
            player->GetPosition() + Vec3::Up() * 100.0f,
            player->GetYaw() + DX_PI_F
        );
    }

    // �G�t�F�N�g�X�V
    gameContext->GetEffectManager().Update(deltaTime);
    StatusUI::SyncStatusBar(L"BossHealthGauge", boss.GetHealthProgress());
    StatusUI::SyncStatusBar(L"BossDownGauge", boss.GetDownGaugeProgress());

    //// �X�e�[�^�XUI�X�V
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
    UIM().RemoveUI(L"BossStatusBackground");
    UIM().RemoveUI(L"BossHealthGauge");
    UIM().RemoveUI(L"BossDownGauge");
}