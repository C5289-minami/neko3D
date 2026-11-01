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
	IAttackSkill* skill = stateMachine_.GetCurrentSkill();
	if (!skill) return;

	skill->BeginPhase(type_);
}

void AttackBaseState::Tick(float deltaTime)
{
	IAttackSkill* skill = stateMachine_.GetCurrentSkill();
	if (!skill)
	{
		stateMachine_.ChangeState(AttackStateType::None);
		return;
	}

	AttackStateType nextType = AttackStateType::None;
	switch (type_)
	{
	case AttackStateType::PreAction:
      skill->UpdatePhase(type_, deltaTime);
		nextType = AttackStateType::Aiming;
		break;
	case AttackStateType::Aiming:
     skill->UpdatePhase(type_, deltaTime);
		nextType = AttackStateType::Attack;
		break;
	case AttackStateType::Attack:
     skill->UpdatePhase(type_, deltaTime);
		nextType = AttackStateType::Recovery;
		break;
	case AttackStateType::Recovery:
       skill->UpdatePhase(type_, deltaTime);
		nextType = AttackStateType::Return;
		break;
	case AttackStateType::Return:
     skill->UpdatePhase(type_, deltaTime);
		break;
	default:
		return;
	}

	skill->UpdateAnimation(deltaTime);

  if (skill->IsPhaseFinished(type_))
	{
		stateMachine_.ChangeState(nextType);
	}
}

void AttackBaseState::Exit()
{
   IAttackSkill* skill = stateMachine_.GetCurrentSkill();
	if (skill)
		skill->FinishPhase(type_);
}

AttackStateMachine::AttackStateMachine()
	: preActionState_(*this, AttackStateType::PreAction),
	aimingState_(*this, AttackStateType::Aiming),
	attackState_(*this, AttackStateType::Attack),
	recoveryState_(*this, AttackStateType::Recovery),
	returnState_(*this, AttackStateType::Return)
{
}
