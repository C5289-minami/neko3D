#include "AttackBaseState.h"
#include "IAttackSkill.h"
#include "AttackStateMachine.h"


AttackBaseState::AttackBaseState(AttackStateMachine& stateMachine, AttackStateType type)
	: stateMachine_(stateMachine),
	type_(type)
{
}

void AttackBaseState::Enter()
{
}

void AttackBaseState::Tick(float deltaTime)
{
	IAttackSkill* skill = stateMachine_.GetCurrentSkill();
	if (!skill)
	{
		stateMachine_.ChangeState(AttackStateType::None);
		return;
	}

	bool phaseFinished = false;
	AttackStateType nextType = AttackStateType::None;
	switch (type_)
	{
	case AttackStateType::PreAction:
		skill->OnPreAction(deltaTime);
		phaseFinished = skill->IsPreActionFinished();
		nextType = AttackStateType::Aiming;
		break;
	case AttackStateType::Aiming:
		skill->OnAiming(deltaTime);
		phaseFinished = skill->IsAimingFinished();
		nextType = AttackStateType::Attack;
		break;
	case AttackStateType::Attack:
		skill->OnAttack(deltaTime);
		phaseFinished = skill->IsAttackFinished();
		nextType = AttackStateType::Recovery;
		break;
	case AttackStateType::Recovery:
		skill->OnRecovery(deltaTime);
		phaseFinished = skill->IsRecoveryFinished();
		nextType = AttackStateType::Return;
		break;
	case AttackStateType::Return:
		skill->OnReturn(deltaTime);
		phaseFinished = skill->IsReturnFinished();
		break;
	default:
		return;
	}

	if (phaseFinished)
	{
		stateMachine_.ChangeState(nextType);
	}
}

void AttackBaseState::Exit()
{
}

AttackStateMachine::AttackStateMachine()
	: preActionState_(*this, AttackStateType::PreAction),
	aimingState_(*this, AttackStateType::Aiming),
	attackState_(*this, AttackStateType::Attack),
	recoveryState_(*this, AttackStateType::Recovery),
	returnState_(*this, AttackStateType::Return)
{
}
