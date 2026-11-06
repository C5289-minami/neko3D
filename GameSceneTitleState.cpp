#include "GameSceneTitleState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "GameContext.h"
#include "UIManager.h"
#include "ResourceKeys.h"
#include "DxPlus/Debug.h"
#include "CameraObjectManager.h"

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
        auto* logo = UIM().CreateImageUI(
            L"Game.Logo",
            ResourceKeys::UI_Logo,
            { DxPlus::CLIENT_WIDTH * 0.05f, DxPlus::CLIENT_HEIGHT * 0.3f },
            {},
            0.5f
        );
		logo->SetCenter({ 0.0f, 0.5f });
    }

    if (!UIM().GetButtonUI(L"Game.StartButton"))
    {
        auto* startButton = UIM().CreateButtonUI(
            L"Game.StartButton",
            ResourceKeys::Button_Red.hover,
            { DxPlus::CLIENT_WIDTH * 0.05f, DxPlus::CLIENT_HEIGHT * 0.65f },
            [this]() { stateMachine_.ChangeState(GameStateType::Select); },
            { -1, 0 },
            0.5f
        );
		startButton->SetButtonHoverSprite(ResourceKeys::Button_Red.back);
        startButton->SetCenter({ 0.0f, 0.5f });
		startButton->SetTextFont(RM().GetFont(ResourceKeys::Font_Game));
		startButton->SetTextScale(1.5f);
        startButton->SetText(L"スタート");

    }

    if (!UIM().GetButtonUI(L"Game.ExitButton"))
    {
        auto* exitButton = UIM().CreateButtonUI(
            L"Game.ExitButton",
            ResourceKeys::Button_Red.hover,
            { DxPlus::CLIENT_WIDTH * 0.05f, DxPlus::CLIENT_HEIGHT * 0.8f },
            []() { PostQuitMessage(0); },
            { -1, 0 },
            0.5f
        );
		exitButton->SetButtonHoverSprite(ResourceKeys::Button_Red.back);
        exitButton->SetCenter({ 0.0f, 0.5f });
		exitButton->SetTextFont(RM().GetFont(ResourceKeys::Font_Game));
		exitButton->SetTextScale(1.5f);
        exitButton->SetText(L"やめる");
    }
	gameScene_.GetCameraDirector().SetPoseImmediate(CObjs().GetCameraPose("Title"));



    selectedIndex_ = 0;

    if (auto* start = UIM().GetButtonUI(L"Game.StartButton"))
        start->SetSelected(true);

    if (auto* exit = UIM().GetButtonUI(L"Game.ExitButton"))
        exit->SetSelected(false);

}
void GameSceneTitleState::Tick(float deltaTime)
{
    DxPlus::Debug::SetFormatString(L"GameSceneTitleState::Tick");
    gameScene_.UpdateStateCamera(deltaTime, "Title");

    using namespace DxPlus::Input;

    if (GetButtonDown(PLAYER1) & BUTTON_UP)
        selectedIndex_ = 0;

    if (GetButtonDown(PLAYER1) & BUTTON_DOWN)
        selectedIndex_ = 1;

    if (auto* start = UIM().GetButtonUI(L"Game.StartButton"))
        start->SetSelected(selectedIndex_ == 0);

    if (auto* exit = UIM().GetButtonUI(L"Game.ExitButton"))
        exit->SetSelected(selectedIndex_ == 1);

    if (GetButtonDown(PLAYER1) & BUTTON_START)
    {
        if (selectedIndex_ == 0)
        {
            if (auto* start = UIM().GetButtonUI(L"Game.StartButton"))
                start->Trigger();
        }
        else
        {
            if (auto* exit = UIM().GetButtonUI(L"Game.ExitButton"))
                exit->Trigger();
        }
    }
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