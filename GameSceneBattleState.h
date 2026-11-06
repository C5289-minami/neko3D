#pragma once
#include "GameBaseState.h"

class GameSceneBattleState : public GameBaseState
{
public:
	GameSceneBattleState(GameStateMachine& gameStateMachine, GameScene& gameScene);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;
private:
    static constexpr float BattleIntroTurnDuration = 1.5f;
	static constexpr float BattleIntroWaitDuration = 1.0f;
	float battleIntroTurnRemaining_{ 0.0f };
	float battleIntroWaitRemaining_{ 0.0f };
	bool battleStarted_{ false };
	bool wasTouchingBossBody_ = false;
	bool wasTailWhipActive_ = false;
};

