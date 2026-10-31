#pragma once
#include "StateMachine.h"
#include "PlayerStateType.h"
#include "PlayerActionEvent.h"
#include <functional>

// State
#include "PlayerIdleState.h"
#include "PlayerWalkState.h"
#include "PlayerGroomingState.h"
#include "PlayerJumpState.h"
#include "PlayerDoubleJumpState.h"
#include "PlayerAttackState.h"
#include "PlayerUltimateAttackState.h"
#include "PlayerDamagingState.h"
#include "PlayerDieState.h"
#include "PlayerClimbingState.h"
#include "PlayerWallJump.h"


class Player;

class PlayerStateMachine : public StateMachine
{
public:
	explicit PlayerStateMachine(Player& player);

	void Initialize();

	void ChangeState(PlayerStateType type);

	PlayerStateType GetCurrentType() const { return currentType_; };


	// 通知先を登録
	void SetEventCallback(
		std::function<void(PlayerActionEvent)> callback);
	// イベントを通知
	void NotifyEvent(PlayerActionEvent event);
private:
	State* FindState(PlayerStateType type);

	Player& player_;
	
	PlayerStateType currentType_{ PlayerStateType::Idle };
	bool initialized_{ false };

	std::function<void(PlayerActionEvent)> eventCallback_;

	PlayerIdleState idleState_;
	PlayerWalkState walkState_;
	PlayerGroomingState groomingState_;
	PlayerJumpState jumpState_;
	PlayerDoubleJumpState doubleJumpState_;
	PlayerAttackState attackState_;
	PlayerUltimateAttackState ultimateAttackState_;
	PlayerDamagingState damagingState_;
	PlayerDieState dieState_;
	PlayerClimbingState climbingState_;
	PlayerWallJump wallJumpState_;
	// 他をここに追加
};
