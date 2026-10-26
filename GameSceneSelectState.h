#pragma once

#include "GameBaseState.h"
#include <vector>

class Player;

class GameSceneSelectState : public GameBaseState
{
public:
    GameSceneSelectState(
        GameStateMachine& gameStateMachine,
        GameScene& gameScene);

    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;

private:
    void SelectIndex(int index);
    void ConfirmSelection();
    int FindPlayerByMouse(int mouseX, int mouseY) const;

    std::vector<Player*> players_;

    int selectedIndex_{ 1 };

    enum class SelectCameraState
    {
        WaitSelectShot,       // Selectショットの完了待ち
        MoveToSelectedPlayer, // 選択中のPlayerの前方へ移動中
        Ready                 // 選択操作可能
    };

    SelectCameraState cameraState_{ SelectCameraState::WaitSelectShot };
    bool confirmAfterCameraMove_{ false };


    bool aWasDown_{ false };
    bool dWasDown_{ false };
    bool enterWasDown_{ false };
    bool mouseWasDown_{ false };
};