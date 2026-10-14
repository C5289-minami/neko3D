#pragma once
#include "StateMachine.h"
#include "PlayerStateType.h"

// State
#include "PlayerIdleState.h"
#include "PlayerWalkState.h"
#include "PlayerKedukuroiState.h"
#include "PlayerNeutralJumpState.h"
#include "PlayerLungingJumpState.h"
#include "PlayerAttackState.h"
#include "PlayerUltimateAttackState.h"
#include "PlayerDamagingState.h"
#include "PlayerDieState.h"

class Player;

class PlayerStateMachine : public StateMachine
{
public:
	explicit PlayerStateMachine(Player& player);

	void Initialize();

	void ChangeState(PlayerStateType type);

	PlayerStateType GetCurrentType() const { return currentType_; };

private:
	State* FindState(PlayerStateType type);

	Player& player_;
	
	PlayerStateType currentType_{ PlayerStateType::Idle };
	bool initialized_{ false };

	PlayerIdleState idleState_;
	PlayerWalkState walkState_;
	PlayerKedukuroiState kedukuroiState_;
	PlayerNeutralJumpState neutralJumpState_;
	PlayerLungingJumpState lungingJumpState_;
	PlayerAttackState attackState_;
	PlayerUltimateAttackState ultimateAttackState_;
	PlayerDamagingState damagingState_;
	PlayerDieState dieState_;
	// ‘¼‚ð‚±‚±‚É’Ç‰Á
};
