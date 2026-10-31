#include "BossPoseSkill.h"

void BossPoseSkill::Reset()
{
	preActionMotion_.Reset();
	aimingMotion_.Reset();
	attackMotion_.Reset();
	recoveryMotion_.Reset();
	returnMotion_.Reset();
}

void BossPoseSkill::OnPreActionEnter()
{
	preActionMotion_.Reset();
	ApplyPhase(AttackStateType::PreAction, 0.0f);
}

void BossPoseSkill::OnAimingEnter()
{
	aimingMotion_.Reset();
	ApplyPhase(AttackStateType::Aiming, 0.0f);
}

void BossPoseSkill::OnAttackEnter()
{
	attackMotion_.Reset();
	ApplyPhase(AttackStateType::Attack, 0.0f);
}

void BossPoseSkill::OnRecoveryEnter()
{
	recoveryMotion_.Reset();
	ApplyPhase(AttackStateType::Recovery, 0.0f);
}

void BossPoseSkill::OnReturnEnter()
{
	returnMotion_.Reset();
	ApplyPhase(AttackStateType::Return, 0.0f);
}

void BossPoseSkill::OnPreAction(float deltaTime)
{
	preActionMotion_.Update(deltaTime);
	ApplyPhase(AttackStateType::PreAction, preActionMotion_.GetEasedProgress());
}

void BossPoseSkill::OnAiming(float deltaTime)
{
	aimingMotion_.Update(deltaTime);
	ApplyPhase(AttackStateType::Aiming, aimingMotion_.GetEasedProgress());
}

void BossPoseSkill::OnAttack(float deltaTime)
{
	attackMotion_.Update(deltaTime);
	ApplyPhase(AttackStateType::Attack, attackMotion_.GetEasedProgress());
}

void BossPoseSkill::OnRecovery(float deltaTime)
{
	recoveryMotion_.Update(deltaTime);
	ApplyPhase(AttackStateType::Recovery, recoveryMotion_.GetEasedProgress());
}

void BossPoseSkill::OnReturn(float deltaTime)
{
	returnMotion_.Update(deltaTime);
	ApplyPhase(AttackStateType::Return, returnMotion_.GetEasedProgress());
}

void BossPoseSkill::SetPhaseDuration(float preAction, float aiming, float attack, float recovery, float returnTime)
{
	preActionMotion_.SetDuration(preAction);
	aimingMotion_.SetDuration(aiming);
	attackMotion_.SetDuration(attack);
	recoveryMotion_.SetDuration(recovery);
	returnMotion_.SetDuration(returnTime);
}
