#include "GameSceneTitleState.h"
#include "GameScene.h"
#include "GameStateMachine.h"
#include "GameContext.h"
#include "UIManager.h"
#include "ResourceKeys.h"
#include "ResourceManager.h"
#include "SoundManager.h"
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
    SoundManager::PlayBGM(RM().GetMusic(ResourceKeys::Music_Title));

    // Logo
    {
        auto* logo = UIM().GetUI(L"Game.Logo");

        if (!logo)
        {
            logo = UIM().CreateImageUI(
                L"Game.Logo",
                ResourceKeys::UI_Logo,
                {
                    DxPlus::CLIENT_WIDTH * 0.05f,
                    DxPlus::CLIENT_HEIGHT * 0.3f
                },
                {},
                0.5f
            );
        }

        if (logo)
        {
            logo->SetCenter({ 0.0f, 0.5f });
            logo->SetFalse();
            logo->Show();
        }
    }

    // Start Button
    {
        auto* startButton = UIM().GetButtonUI(L"Game.StartButton");

        if (!startButton)
        {
            startButton = UIM().CreateButtonUI(
                L"Game.StartButton",
                ResourceKeys::Button_Red.back,
                {
                    DxPlus::CLIENT_WIDTH * 0.05f,
                    DxPlus::CLIENT_HEIGHT * 0.65f
                },
                [this]()
                {
                    SoundManager::PlaySE(
                        RM().GetSound(ResourceKeys::Sound_MenuConfirm)
                    );

                    stateMachine_.ChangeState(GameStateType::Select);
                },
                { -1.0f, 0.0f },
                0.5f
            );

            if (startButton)
            {
                startButton->SetButtonHoverSprite(
                    ResourceKeys::Button_Red.hover
                );
                startButton->SetCenter({ 0.0f, 0.5f });
                startButton->SetTextFont(
                    RM().GetFont(ResourceKeys::Font_Game)
                );
                startButton->SetTextScale(1.5f);
                startButton->SetText(L"スタート");
            }
        }

        if (startButton)
        {
            startButton->SetFalse();
            startButton->Show();
        }
    }

    // Exit Button
    {
        auto* exitButton = UIM().GetButtonUI(L"Game.ExitButton");

        if (!exitButton)
        {
            exitButton = UIM().CreateButtonUI(
                L"Game.ExitButton",
                ResourceKeys::Button_Red.back,
                {
                    DxPlus::CLIENT_WIDTH * 0.05f,
                    DxPlus::CLIENT_HEIGHT * 0.8f
                },
                []()
                {
                    PostQuitMessage(0);
                },
                { -1.0f, 0.0f },
                0.5f
            );

            if (exitButton)
            {
                exitButton->SetButtonHoverSprite(
                    ResourceKeys::Button_Red.hover
                );
                exitButton->SetCenter({ 0.0f, 0.5f });
                exitButton->SetTextFont(
                    RM().GetFont(ResourceKeys::Font_Game)
                );
                exitButton->SetTextScale(1.5f);
                exitButton->SetText(L"やめる");
            }
        }

        if (exitButton)
        {
            exitButton->SetFalse();
            exitButton->Show();
        }
    }
	gameScene_.GetCameraDirector().SetPoseImmediate(CObjs().GetCameraPose("Title"));

    auto& previews = gameScene_.GetGameContext()->GetSelectPreviewPlayers();
    previews[0].PlayAnimationFromModel(ResourceKeys::Model_PlayerAnim_SelectMike);
    previews[1].PlayAnimationFromModel(ResourceKeys::Model_PlayerAnim_SelectSabi);


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

    auto& previews =
        gameScene_.GetGameContext()->GetSelectPreviewPlayers();

    previews[0].UpdateAnimation(deltaTime);
    previews[1].UpdateAnimation(deltaTime);

    auto* start = UIM().GetButtonUI(L"Game.StartButton");
    auto* exit = UIM().GetButtonUI(L"Game.ExitButton");

    if (!start || !exit)
        return;

    // 表示モーションが終わるまで操作させない。
    if (start->GetCurrentState() != UIBase::UIState::Normal ||
        exit->GetCurrentState() != UIBase::UIState::Normal)
    {
        return;
    }

    using namespace DxPlus::Input;

    if (GetButtonDown(PLAYER1) & BUTTON_UP)
        selectedIndex_ = 0;

    if (GetButtonDown(PLAYER1) & BUTTON_DOWN)
        selectedIndex_ = 1;

    start->SetSelected(selectedIndex_ == 0);
    exit->SetSelected(selectedIndex_ == 1);

    if (GetButtonDown(PLAYER1) & BUTTON_START)
    {
        if (selectedIndex_ == 0)
            start->Trigger();
        else
            exit->Trigger();
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