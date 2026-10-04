#include "AttackStateMachine.h"


void AttackStateMachine::Initialize()
{
	ChangeState(AttackStateType::None);
}

void AttackStateMachine::StartAttack(IAttackSkill* skill)
{
	Initialize();
	// スキルがなければ終了
	if (!skill)
	{
		return;
	}

	currentSkill_ = skill;
	finished_ = false;
	ChangeState(AttackStateType::PreAction);
}

void AttackStateMachine::ChangeState(AttackStateType type)
{
	// もしNoneに変更する場合は、状態を解除して終了
	if (type == AttackStateType::None)
	{
		StateMachine::SwitchState(nullptr);
		currentType_ = AttackStateType::None;
		currentSkill_ = nullptr;
		finished_ = true;
		return;
	}

	if (currentType_ == type) return;
	State* newState = FindState(type);
	if (!newState) return;
	currentType_ = type;
	finished_ = false;
	StateMachine::SwitchState(newState);
}

State* AttackStateMachine::FindState(AttackStateType type)
{
	switch (type)
	{
	case AttackStateType::None:
		return nullptr;
	case AttackStateType::PreAction:
		return &preActionState_;
	case AttackStateType::Aiming:
		return &aimingState_;
	case AttackStateType::Attack:
		return &attackState_;
	case AttackStateType::Recovery:
		return &recoveryState_;
	case AttackStateType::Return:
		return &returnState_;
	default:
		return nullptr;
	}
}

