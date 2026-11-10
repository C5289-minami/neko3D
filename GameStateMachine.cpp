#include "GameStateMachine.h"
#include "ResourceKeys.h"
#include "ResourceManager.h"
#include "SoundManager.h"

GameStateMachine::GameStateMachine(GameScene& gameScene)
    : gameScene_(gameScene),
    currentType_(GameStateType::Title),
    titleState_(*this, gameScene),
    selectState_(*this, gameScene),
    tutorialState_(*this, gameScene),
    battleState_(*this, gameScene),
    clearState_(*this, gameScene),
	gameOverState_(*this, gameScene)
{

}
void GameStateMachine::Initialize()
{
    initialized_ = false;

    ChangeState(GameStateType::Title);
}

void GameStateMachine::End()
{
    SwitchState(nullptr);
    initialized_ = false;
}

void GameStateMachine::ChangeState(GameStateType type)
{
    if (initialized_ && currentType_ == type) return;

    State* newState = FindState(type);
    if (!newState)return;

    if (type == GameStateType::Clear || type == GameStateType::GameOver)
    {
        SoundManager::StopBGM(RM().GetMusic(ResourceKeys::Music_Title));
        SoundManager::StopBGM(RM().GetMusic(ResourceKeys::Music_Battle));
    }

    currentType_ = type;
    initialized_ = true;
    StateMachine::SwitchState(newState);

}


void GameStateMachine::StartBattleWithoutIntro()
{
    ChangeState(GameStateType::Battle);
    battleState_.SkipIntro();
}

State* GameStateMachine::FindState(GameStateType type)
{
    switch (type)
    {
    case GameStateType::Title:
        return &titleState_;
    case GameStateType::Select:
        return &selectState_;
    case GameStateType::Tutorial:
        return &tutorialState_;
    case GameStateType::Battle:
        return &battleState_;
    case GameStateType::Clear:
        return &clearState_;
    case GameStateType::GameOver:
		return &gameOverState_;
    default:
        return nullptr;
    }
}
