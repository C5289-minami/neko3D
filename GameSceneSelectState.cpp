#include "GameSceneSelectState.h"

#include "GameScene.h"
#include "GameStateMachine.h"
#include "GameContext.h"
#include "Player.h"
#include "ResourceKeys.h"
#include "DxConv.h"
#include "DxPlus/Debug.h"
#include "UIManager.h"
#include "ButtonLabel.h"
#include "ResourceManager.h"
#include "SoundManager.h"

#include <array>
#include <cmath>
#include <memory>
#include <string>

namespace
{
    constexpr float SelectTargetH = 180.0f;
    constexpr float SelectTargetRightOffset = 150.0f;
    constexpr float SelectCameraLeftOffset = 120.0f;
    constexpr float SelectCameraForwardDistance = 350.0f;
    constexpr float SelectCameraMoveDuration = 0.6f;

    constexpr const wchar_t* SelectHandAnimatorId =
        L"Select.HandAnimation";
    constexpr float SelectHandXRate = 0.36f;
    constexpr float SelectHandYRate = 0.72f;

    DxPlus::Vec2 GetSelectHandPosition()
    {
        return {
            DxPlus::CLIENT_WIDTH *
                SelectHandXRate,

            DxPlus::CLIENT_HEIGHT *
                SelectHandYRate 
        };
    }

    struct SelectCard
    {
        const wchar_t* centerUIKey;
        const wchar_t* leftUIKey;
        const wchar_t* rightUIKey;

        const wchar_t* centerSpriteKey;
        const wchar_t* leftSpriteKey;
        const wchar_t* rightSpriteKey;

        const wchar_t* soundKey;
        const char* statusName;
    };

    constexpr std::array<SelectCard, 3>
        SelectCards{ {
        {
            L"Select.Mike",
            L"Select.Left.Mike",
            L"Select.Right.Mike",
            ResourceKeys::SelectPlayers.mike,
            ResourceKeys::SelectAKeys.mike,
            ResourceKeys::SelectDKeys.mike,
            ResourceKeys::SoundSelectPlayers.mike,
            u8"たか"
        },
        {
            L"Select.Sabi",
            L"Select.Left.Sabi",
            L"Select.Right.Sabi",
            ResourceKeys::SelectPlayers.sabi,
            ResourceKeys::SelectAKeys.sabi,
            ResourceKeys::SelectDKeys.sabi,
            ResourceKeys::SoundSelectPlayers.sabi,
            u8"ぎし"
        },
        {
            L"Select.Kuro",
            L"Select.Left.Kuro",
            L"Select.Right.Kuro",
            ResourceKeys::SelectPlayers.kuro,
            ResourceKeys::SelectAKeys.kuro,
            ResourceKeys::SelectDKeys.kuro,
            ResourceKeys::SoundSelectPlayers.kuro,
            u8"ゆう"
        }
    } };

    class SelectCommonUI : public UIBase
    {
    public:
        explicit SelectCommonUI(
            DxPlus::Vec2 position)
            : UIBase(
                ResourceKeys::SelectCommon,
                position,
                { 0.0f, -1.0f },
                SelectCameraMoveDuration)
        {
            SetCenter({
                0.5f,
                0.5f
                });
        }
    };

    Camera::CameraPose MakeCameraPoseInFrontOfPlayer(
        Camera::CameraPose pose,
        const Player& player)
    {
        const Vec3 playerPos =
            player.GetWorldPosition();

        const float yaw =
            player.GetYaw();

        const Vec3 forward{
            std::sin(yaw),
            0.0f,
            std::cos(yaw)
        };

        const Vec3 right{
            std::cos(yaw),
            0.0f,
            -std::sin(yaw)
        };

        // Targetを画面右側に配置するため、
        // 注視点を右へずらす。
        const Vec3 target =
            playerPos +
            Vec3::Up() * SelectTargetH +
            right * SelectTargetRightOffset;

        // キャラクターの正面側へカメラを配置する。
        pose.position =
            target +
            forward * SelectCameraForwardDistance;

        // カメラを少し左へ移動する。
        pose.position -=
            right * SelectCameraLeftOffset;

        const Vec3 direction =
            target - pose.position;

        pose.yaw = std::atan2(
            direction.x,
            direction.z
        );

        pose.pitch = std::atan2(
            direction.y,
            std::sqrt(
                direction.x * direction.x +
                direction.z * direction.z
            )
        );

        return pose;
    }
}

// ============================================================
// Construction
// ============================================================

GameSceneSelectState::GameSceneSelectState(
    GameStateMachine& gameStateMachine,
    GameScene& gameScene)
    : GameBaseState(
        gameStateMachine,
        gameScene)
{
}

// ============================================================
// Enter
// ============================================================

void GameSceneSelectState::Enter()
{
    DxPlus::Debug::SetFormatString(
        L"GameSceneSelectState::Enter"
    );

    const DxPlus::Vec2 position{
        DxPlus::CLIENT_WIDTH * 0.25f,
        DxPlus::CLIENT_HEIGHT * 0.5f
    };

    // ========================================================
    // 中央UI
    // ========================================================

    for (const auto& card : SelectCards)
    {
        if (!UIM().GetUI(card.centerUIKey))
        {
            UIM().CreateImageUI(
                card.centerUIKey,
                card.centerSpriteKey,
                position,
                { 0.0f, -1.0f },
                SelectCameraMoveDuration
            );
        }

        if (auto* ui =
            UIM().GetUI(card.centerUIKey))
        {
            ui->SetFalse();
        }
    }

    // ========================================================
    // 左右UI
    // ========================================================

    float offset{};

    auto* centerSprite =
        RM().GetSprite(
            SelectCards[0].centerSpriteKey
        );

    if (centerSprite)
    {
        DxLib::GetGraphSizeF(
            centerSprite->GetID(),
            &offset,
            nullptr
        );

        offset *= 0.55f;
    }
    else
    {
        offset = 100.0f;
    }

    const DxPlus::Vec2 rightPosition{
        position.x + offset,
        position.y
    };

    const DxPlus::Vec2 leftPosition{
        position.x - offset,
        position.y
    };

    for (const auto& card : SelectCards)
    {
        if (!UIM().GetUI(card.leftUIKey))
        {
            UIM().CreateImageUI(
                card.leftUIKey,
                card.leftSpriteKey,
                leftPosition,
                { 1.0f, 0.0f },
                SelectCameraMoveDuration
            );
        }

        if (!UIM().GetUI(card.rightUIKey))
        {
            UIM().CreateImageUI(
                card.rightUIKey,
                card.rightSpriteKey,
                rightPosition,
                { -1.0f, 0.0f },
                SelectCameraMoveDuration
            );
        }

        if (auto* ui =
            UIM().GetUI(card.leftUIKey))
        {
            ui->SetFalse();
        }

        if (auto* ui =
            UIM().GetUI(card.rightUIKey))
        {
            ui->SetFalse();
        }
    }

    // ========================================================
    // 共通UI
    // ========================================================

    if (!UIM().GetUI(L"Select.Common"))
    {
        UIM().RegisterUI(
            L"Select.Common",
            std::make_unique<SelectCommonUI>(
                position +
                DxPlus::Vec2{
                    0.0f,
                    220.0f
                }
            )
        );
    }

    if (auto* commonUI =
        UIM().GetUI(L"Select.Common"))
    {
        commonUI->SetFalse();
    }

    // ========================================================
    // GameContext
    // ========================================================

    auto* context =
        gameScene_.GetGameContext();

    if (!context)
    {
        return;
    }

    // UIM().Clear()後の場合も手UIを再生成する。
    context->HandUIInit();

    handAnimation_ = nullptr;


    // ========================================================
    // Preview Player
    // ========================================================

    players_.clear();

    auto& previews =
        context->GetSelectPreviewPlayers();

    for (std::size_t i = 0;
        i < previews.size();
        ++i)
    {
        auto& preview = previews[i];

        preview.SetGravityEnabled(false);
        preview.SetDefMove(false);
        preview.StopMove();

        if (i == 0)
        {
            preview.PlayAnimationFromModel(
                ResourceKeys::
                Model_PlayerAnim_SelectMike
            );
        }
        else if (i == 1)
        {
            preview.PlayAnimationFromModel(
                ResourceKeys::
                Model_PlayerAnim_SelectSabi
            );
        }

        players_.push_back(&preview);
    }

    selectedIndex_ = 1;

    cameraState_ =
        SelectCameraState::WaitSelectShot;

    confirmAfterCameraMove_ = false;

    aWasDown_ = false;
    dWasDown_ = false;
    enterWasDown_ = false;

}

// ============================================================
// Tick
// ============================================================

void GameSceneSelectState::Tick(
    float deltaTime)
{
    DxPlus::Debug::SetFormatString(
        L"GameSceneSelectState::Tick"
    );

    if (players_.empty())
    {
        return;
    }

    for (auto* player : players_)
    {
        if (player)
        {
            player->UpdateAnimation(deltaTime);
        }
    }

    /*
        TwoFrameUIAnimatorのUpdateは、
        UIManager::Updateで自動的に呼ばれる。

        ここではUpdateしない。
    */

    auto& director =
        gameScene_.GetCameraDirector();

    const bool transitionCompleted =
        gameScene_.UpdateStateCamera(
            deltaTime,
            "Select"
        );

    // ========================================================
    // カメラ状態
    // ========================================================

    switch (cameraState_)
    {
    case SelectCameraState::WaitSelectShot:
    {
        if (transitionCompleted &&
            !director.IsTransitioning())
        {
            SoundManager::PlaySE(
                RM().GetSound(
                    SelectCards[
                        selectedIndex_
                    ].soundKey
                )
            );

            const auto pose =
                MakeCameraPoseInFrontOfPlayer(
                    director.GetCurrentPose(),
                    *players_[selectedIndex_]
                );

            director.MoveTo(
                pose,
                SelectCameraMoveDuration,
                Motion::Easing::EaseInOut
            );

            cameraState_ =
                SelectCameraState::
                MoveToSelectedPlayer;
        }

        break;
    }

    case SelectCameraState::MoveToSelectedPlayer:
    {
        if (transitionCompleted &&
            !director.IsTransitioning())
        {
            cameraState_ =
                SelectCameraState::Ready;

            UpdateSelectedUI();

            if (confirmAfterCameraMove_)
            {
                confirmAfterCameraMove_ = false;

                ConfirmSelection();
                return;
            }
        }

        break;
    }

    case SelectCameraState::Ready:
        break;
    }

    // ========================================================
    // 入力
    // ========================================================

    const bool aDown =
        DxLib::CheckHitKey(
            KEY_INPUT_A
        ) != 0;

    const bool dDown =
        DxLib::CheckHitKey(
            KEY_INPUT_D
        ) != 0;

    const bool enterDown =
        DxLib::CheckHitKey(
            KEY_INPUT_RETURN
        ) != 0;


    const bool aPressed =
        aDown && !aWasDown_;

    const bool dPressed =
        dDown && !dWasDown_;

    const bool enterPressed =
        enterDown && !enterWasDown_;

    aWasDown_ = aDown;
    dWasDown_ = dDown;
    enterWasDown_ = enterDown;

    if (cameraState_ !=
        SelectCameraState::Ready)
    {
        return;
    }

    // ========================================================
    // 選択
    // ========================================================

    if (aPressed)
    {
        SelectIndex(
            selectedIndex_ - 1
        );

        return;
    }

    if (dPressed)
    {
        SelectIndex(
            selectedIndex_ + 1
        );

        return;
    }

    // ========================================================
    // 決定
    // ========================================================

    if (enterPressed)
    {
        ConfirmSelection();
    }
}

// ============================================================
// Select
// ============================================================

void GameSceneSelectState::SelectIndex(
    int index)
{
    if (players_.empty())
    {
        return;
    }

    const int count =
        static_cast<int>(
            players_.size()
            );

    index =
        (index % count + count) % count;

    if (index == selectedIndex_)
    {
        return;
    }

    SoundManager::StopSE(
        RM().GetSound(
            SelectCards[
                selectedIndex_
            ].soundKey
        )
    );

    // 現在表示している手を停止する。
    if (handAnimation_)
    {
        handAnimation_->Stop();
    }

    selectedIndex_ = index;

    SoundManager::PlaySE(
        RM().GetSound(
            SelectCards[
                selectedIndex_
            ].soundKey
        )
    );

    for (const auto& card : SelectCards)
    {
        if (auto* ui =
            UIM().GetUI(card.centerUIKey))
        {
            ui->Hide();
        }

        if (auto* ui =
            UIM().GetUI(card.leftUIKey))
        {
            ui->Hide();
        }

        if (auto* ui =
            UIM().GetUI(card.rightUIKey))
        {
            ui->Hide();
        }
    }

    if (auto* ui =
        UIM().GetUI(L"Select.Common"))
    {
        ui->Hide();
    }

    auto& director =
        gameScene_.GetCameraDirector();

    const auto pose =
        MakeCameraPoseInFrontOfPlayer(
            director.GetCurrentPose(),
            *players_[selectedIndex_]
        );

    director.MoveTo(
        pose,
        SelectCameraMoveDuration,
        Motion::Easing::EaseInOut
    );

    cameraState_ =
        SelectCameraState::
        MoveToSelectedPlayer;
}

// ============================================================
// Confirm
// ============================================================

void GameSceneSelectState::ConfirmSelection()
{
    if (players_.empty())
    {
        return;
    }

    auto* context =
        gameScene_.GetGameContext();

    if (!context)
    {
        return;
    }

    Player& selected =
        *players_[selectedIndex_];

    const auto& card =
        SelectCards[selectedIndex_];

    const auto* status =
        CharSettingsManager::GetByName(
            card.statusName
        );

    selected.SetCharData(
        status
        ? *status
        : CharData{}
    );

    // Basketとの親子関係を解除する。
    selected.ClearOwner();

    // 本編で使用するPlayerを切り替える。
    context->SetActivePlayer(selected);

    auto* player =
        context->GetPlayer();

    if (!player)
    {
        return;
    }

    player->SetGravityEnabled(true);

    player->GetModelObject().SetRotation({});

    player->SetRotation({
        0.0f,
        60.0f * DxPlus::Deg2Rad,
        0.0f
        });

    player->GetModelObject().SetScale({
        1.0f,
        1.0f,
        1.0f
        });

    player->GetModelObject().SetPosition({});

    stateMachine_.ChangeState(
        GameStateType::Tutorial
    );
}

// ============================================================
// Selected UI
// ============================================================

void GameSceneSelectState::UpdateSelectedUI()
{
    const int count =
        static_cast<int>(
            SelectCards.size()
            );

    const int leftIndex =
        (selectedIndex_ - 1 + count) %
        count;

    const int rightIndex =
        (selectedIndex_ + 1) %
        count;

    for (std::size_t i = 0;
        i < SelectCards.size();
        ++i)
    {
        if (auto* ui =
            UIM().GetUI(
                SelectCards[i].centerUIKey
            ))
        {
            if (i ==
                static_cast<std::size_t>(
                    selectedIndex_
                    ))
            {
                ui->Show();
            }
            else
            {
                ui->SetFalse();
            }
        }

        if (auto* ui =
            UIM().GetUI(
                SelectCards[i].leftUIKey
            ))
        {
            if (i ==
                static_cast<std::size_t>(
                    leftIndex
                    ))
            {
                ui->Show();
            }
            else
            {
                ui->SetFalse();
            }
        }

        if (auto* ui =
            UIM().GetUI(
                SelectCards[i].rightUIKey
            ))
        {
            if (i ==
                static_cast<std::size_t>(
                    rightIndex
                    ))
            {
                ui->Show();
            }
            else
            {
                ui->SetFalse();
            }
        }
    }

    if (auto* ui =
        UIM().GetUI(L"Select.Common"))
    {
        ui->Show();
    }

    UpdateHandAnimation();
}

void GameSceneSelectState::UpdateHandAnimation()
{
    auto* context =
        gameScene_.GetGameContext();

    if (!context)
    {
        return;
    }

    GameContext::PlayerHandType handType{};

    switch (selectedIndex_)
    {
    case 0:
        handType =
            GameContext::PlayerHandType::Mike;
        break;

    case 1:
        handType =
            GameContext::PlayerHandType::Sabi;
        break;

    case 2:
        handType =
            GameContext::PlayerHandType::Kuro;
        break;

    default:
        return;
    }

    handAnimation_ =
        context->StartPlayerHandAnimation(
            SelectHandAnimatorId,
            handType,
            GetSelectHandPosition(),
            true // X反転
        );
}

// ============================================================
// Exit
// ============================================================

void GameSceneSelectState::Exit()
{
    DxPlus::Debug::SetFormatString(
        L"GameSceneSelectState::Exit"
    );

    if (auto* context =
        gameScene_.GetGameContext())
    {
        context->StopPlayerHandAnimation(
            SelectHandAnimatorId
        );
    }

    handAnimation_ = nullptr;

    SoundManager::StopSE(
        RM().GetSound(
            SelectCards[
                selectedIndex_
            ].soundKey
        )
    );

    for (const auto& card : SelectCards)
    {
        if (auto* ui =
            UIM().GetUI(card.centerUIKey))
        {
            ui->SetFalse();
        }

        if (auto* ui =
            UIM().GetUI(card.leftUIKey))
        {
            ui->SetFalse();
        }

        if (auto* ui =
            UIM().GetUI(card.rightUIKey))
        {
            ui->SetFalse();
        }
    }

    if (auto* ui =
        UIM().GetUI(L"Select.Common"))
    {
        ui->SetFalse();
    }

}