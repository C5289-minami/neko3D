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

namespace
{
    constexpr const wchar_t* TitleHandAnimatorId = L"Title.Hand";
}

GameSceneTitleState::GameSceneTitleState(GameStateMachine& gameStateMachine, GameScene& gameScene)
    : GameBaseState(gameStateMachine, gameScene)
{
}

void GameSceneTitleState::Enter()
{
    DxPlus::Debug::SetFormatString(L"GameSceneTitleState::Enter");
    DxLib::SetMouseDispFlag(TRUE);
    SoundManager::PlayBGM(RM().GetMusic(ResourceKeys::Music_Title));

    {
        auto* logo = UIM().GetUI(L"Game.Logo");
        if (!logo)
        {
            logo = UIM().CreateImageUI(L"Game.Logo", ResourceKeys::UI_Logo,
                { DxPlus::CLIENT_WIDTH * 0.05f, DxPlus::CLIENT_HEIGHT * 0.3f }, {}, 0.5f);
        }
        if (logo)
        {
            logo->SetCenter({ 0.0f, 0.5f });
            logo->SetFalse();
            logo->Show();
        }
    }

    {
        auto* creditImage = UIM().GetUI(L"Game.CreditImage");
        if (!creditImage)
        {
            creditImage = UIM().CreateImageUI(L"Game.CreditImage", ResourceKeys::UI_CreditImage,
                { DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.5f });
            if (creditImage)
            {
                creditImage->SetCenter({ 0.5f, 0.5f });
                creditImage->SetZOrder(10);
            }
        }
        if (creditImage)
            creditImage->SetFalse();
    }

    auto createButton = [this](const wchar_t* id, const wchar_t* text, float y,
        std::function<void()> callback)
    {
        auto* button = UIM().GetButtonUI(id);
        if (!button)
        {
            button = UIM().CreateButtonUI(id, ResourceKeys::Button_Red.back,
                { DxPlus::CLIENT_WIDTH * 0.05f, DxPlus::CLIENT_HEIGHT * y },
                std::move(callback), { -1.0f, 0.0f }, 0.5f);
            if (button)
            {
                button->SetButtonHoverSprite(ResourceKeys::Button_Red.hover);
                button->SetCenter({ 0.0f, 0.5f });
                button->SetTextFont(RM().GetFont(ResourceKeys::Font_Game));
                button->SetTextScale(1.5f);
                button->SetText(text);
            }
        }
        if (button)
        {
            button->SetFalse();
            button->Show();
        }
    };

    createButton(L"Game.StartButton", L"スタート", 0.6f, [this]()
    {
        SoundManager::PlaySE(RM().GetSound(ResourceKeys::Sound_MenuConfirm));
        stateMachine_.ChangeState(GameStateType::Select);
    });

    createButton(L"Game.CreditButton", L"クレジット", 0.72f, []()
    {
        SoundManager::PlaySE(RM().GetSound(ResourceKeys::Sound_MenuConfirm));
        if (auto* image = UIM().GetUI(L"Game.CreditImage"))
        {
            if (image->IsVisible()) image->SetFalse();
            else image->SetTrue();
        }
    });

    createButton(L"Game.ExitButton", L"やめる", 0.84f, []()
    {
        PostQuitMessage(0);
    });

    gameScene_.GetCameraDirector().SetPoseImmediate(CObjs().GetCameraPose("Title"));
    auto& previews = gameScene_.GetGameContext()->GetSelectPreviewPlayers();
    previews[0].PlayAnimationFromModel(ResourceKeys::Model_PlayerAnim_SelectMike);
    previews[1].PlayAnimationFromModel(ResourceKeys::Model_PlayerAnim_SelectSabi);

    selectedIndex_ = 0;
    if (auto* start = UIM().GetButtonUI(L"Game.StartButton")) start->SetSelected(true);
    if (auto* credit = UIM().GetButtonUI(L"Game.CreditButton")) credit->SetSelected(false);
    if (auto* exit = UIM().GetButtonUI(L"Game.ExitButton")) exit->SetSelected(false);

    if (auto* context = gameScene_.GetGameContext())
    {
        context->StartPlayerHandAnimation(TitleHandAnimatorId,
            GameContext::PlayerHandType::Mike,
            { DxPlus::CLIENT_WIDTH * 0.38f, DxPlus::CLIENT_HEIGHT * 0.65f }, true);
    }
}

void GameSceneTitleState::Tick(float deltaTime)
{
    DxPlus::Debug::SetFormatString(L"GameSceneTitleState::Tick");
    gameScene_.UpdateStateCamera(deltaTime, "Title");
    auto& previews = gameScene_.GetGameContext()->GetSelectPreviewPlayers();
    previews[0].UpdateAnimation(deltaTime);
    previews[1].UpdateAnimation(deltaTime);

    auto* start = UIM().GetButtonUI(L"Game.StartButton");
    auto* credit = UIM().GetButtonUI(L"Game.CreditButton");
    auto* exit = UIM().GetButtonUI(L"Game.ExitButton");
    if (!start || !credit || !exit) return;

    if (start->GetCurrentState() != UIBase::UIState::Normal ||
        credit->GetCurrentState() != UIBase::UIState::Normal ||
        exit->GetCurrentState() != UIBase::UIState::Normal) return;

    using namespace DxPlus::Input;
    if (GetButtonDown(PLAYER1) & BUTTON_DOWN)
        selectedIndex_ = (selectedIndex_ + 1) % 3;
    if (GetButtonDown(PLAYER1) & BUTTON_UP)
        selectedIndex_ = (selectedIndex_ + 2) % 3;

    start->SetSelected(selectedIndex_ == 0);
    credit->SetSelected(selectedIndex_ == 1);
    exit->SetSelected(selectedIndex_ == 2);

    if (selectedIndex_ != 1)
    {
        if (auto* creditImage = UIM().GetUI(L"Game.CreditImage"))
            creditImage->SetFalse();
    }

    const DxPlus::Vec2 handPosition{
        DxPlus::CLIENT_WIDTH * 0.38f,
        selectedIndex_ == 0 ? DxPlus::CLIENT_HEIGHT * 0.6f :
        selectedIndex_ == 1 ? DxPlus::CLIENT_HEIGHT * 0.72f :
                              DxPlus::CLIENT_HEIGHT * 0.84f
    };
    if (auto* animator = UIM().GetTwoFrameAnimator(TitleHandAnimatorId))
        animator->SetAnchorPosition(handPosition);

    if (GetButtonDown(PLAYER1) & BUTTON_START)
    {
        if (selectedIndex_ == 0) start->Trigger();
        else if (selectedIndex_ == 1) credit->Trigger();
        else exit->Trigger();
    }
}

void GameSceneTitleState::Exit()
{
    DxPlus::Debug::SetFormatString(L"GameSceneTitleState::Exit");
    DxLib::SetMouseDispFlag(FALSE);
    if (auto* logo = UIM().GetUI(L"Game.Logo")) logo->Hide();
    if (auto* start = UIM().GetButtonUI(L"Game.StartButton")) start->Hide();
    if (auto* credit = UIM().GetButtonUI(L"Game.CreditButton")) credit->Hide();
    if (auto* exit = UIM().GetButtonUI(L"Game.ExitButton")) exit->Hide();
    if (auto* creditImage = UIM().GetUI(L"Game.CreditImage")) creditImage->SetFalse();
    if (auto* context = gameScene_.GetGameContext())
        context->StopPlayerHandAnimation(TitleHandAnimatorId);
}