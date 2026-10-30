#include "GameSceneTitleState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "GameContext.h"
#include "UIManager.h"
#include "ResourceKeys.h"
#include "DxPlus/Debug.h"

GameSceneTitleState::GameSceneTitleState(
    GameStateMachine& gameStateMachine,
    GameScene& gameScene)
    : GameBaseState(gameStateMachine, gameScene)
{
}

void GameSceneTitleState::Enter()
{
    DxPlus::Debug::SetFormatString(L"GameSceneTitleState::Enter");
    DxLib::SetMouseDispFlag(TRUE);

    // タイトルUIを再表示
    if (auto* logo = UIM().GetUI(L"Game.Logo"))
        logo->SetTrue();

    if (auto* start = UIM().GetButtonUI(L"Game.StartButton"))
        start->SetTrue();

    if (auto* exit = UIM().GetButtonUI(L"Game.ExitButton"))
        exit->SetTrue();

    // 存在しない場合だけ生成
    if (!UIM().GetUI(L"Game.Logo"))
    {
        UIM().CreateImageUI(
            L"Game.Logo",
            ResourceKeys::UI_Logo,
            { DxPlus::CLIENT_WIDTH * 0.3f, DxPlus::CLIENT_HEIGHT * 0.3f },
            {},
            0.5f
        );
    }

    if (!UIM().GetButtonUI(L"Game.StartButton"))
    {
        UIM().CreateButtonUI(
            L"Game.StartButton",
            ResourceKeys::UI_StartButton,
            { DxPlus::CLIENT_WIDTH * 0.2f, DxPlus::CLIENT_HEIGHT * 0.6f },
            [this]() { stateMachine_.ChangeState(GameStateType::Select); },
            { -1, 0 },
            0.5f
        );
    }

    if (!UIM().GetButtonUI(L"Game.ExitButton"))
    {
        UIM().CreateButtonUI(
            L"Game.ExitButton",
            ResourceKeys::UI_ExitButton,
            { DxPlus::CLIENT_WIDTH * 0.2f, DxPlus::CLIENT_HEIGHT * 0.75f },
            []() { PostQuitMessage(0); },
            { -1, 0 },
            0.5f
        );
    }
}
void GameSceneTitleState::Tick(float deltaTime)
{
    DxPlus::Debug::SetFormatString(L"GameSceneTitleState::Tick");
    gameScene_.UpdateStateCamera(deltaTime, "Title");
}

void GameSceneTitleState::Exit()
{
    DxPlus::Debug::SetFormatString(L"GameSceneTitleState::Exit");
    DxLib::SetMouseDispFlag(FALSE);

    if (auto* logo = UIM().GetUI(L"Game.Logo"))
        logo->Hide();

    if (auto* start = UIM().GetButtonUI(L"Game.StartButton"))
        start->Hide();

    if (auto* exit = UIM().GetButtonUI(L"Game.ExitButton"))
        exit->Hide();
}