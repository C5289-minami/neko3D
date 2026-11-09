#pragma once

#include "GameBaseState.h"
#include "TwoFrameUIAnimator.h"

#include <vector>

class Player;

class GameSceneSelectState : public GameBaseState
{
public:
    GameSceneSelectState(
        GameStateMachine& gameStateMachine,
        GameScene& gameScene
    );

    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;

private:
    void SelectIndex(int index);
    void ConfirmSelection();

    void UpdateSelectedUI();
    void UpdateHandAnimation();

    std::vector<Player*> players_;

    int selectedIndex_{ 1 };

    enum class SelectCameraState
    {
        WaitSelectShot,
        MoveToSelectedPlayer,
        Ready
    };

    SelectCameraState cameraState_{
        SelectCameraState::WaitSelectShot
    };

    bool confirmAfterCameraMove_{ false };

    bool aWasDown_{ false };
    bool dWasDown_{ false };
    bool enterWasDown_{ false };

    TwoFrameUIAnimator* handAnimation_{};
};