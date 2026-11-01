#include "BossPoseSkill.h"

void BossPoseSkill::Reset()
{
	preActionMotion_.Reset();
	aimingMotion_.Reset();
	attackMotion_.Reset();
	recoveryMotion_.Reset();
	returnMotion_.Reset();
}

void BossPoseSkill::BeginPhase(AttackStateType phase)
{
	GetPhaseMotion(phase).Reset();
	EnterPhase(phase);
}

void BossPoseSkill::UpdatePhase(AttackStateType phase, float deltaTime)
{
    Motion& motion = GetPhaseMotion(phase);
	motion.Update(deltaTime);
	ApplyPhase(phase, motion.GetEasedProgress());
}

void BossPoseSkill::FinishPhase(AttackStateType phase)
{
	EndPhase(phase);
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

bool BossPoseSkill::IsPreActionFinished() const
{
	return IsPhaseFinished(AttackStateType::PreAction);
}

bool BossPoseSkill::IsAimingFinished() const
{
	return IsPhaseFinished(AttackStateType::Aiming);
}

bool BossPoseSkill::IsAttackFinished() const
{
	return IsPhaseFinished(AttackStateType::Attack);
}

bool BossPoseSkill::IsRecoveryFinished() const
{
	return IsPhaseFinished(AttackStateType::Recovery);
}

bool BossPoseSkill::IsReturnFinished() const
{
	return IsPhaseFinished(AttackStateType::Return);
}


void BossPoseSkill::SetPhaseDuration(float preAction, float aiming, float attack, float recovery, float returnTime)
{
	preActionMotion_.SetDuration(preAction);
	aimingMotion_.SetDuration(aiming);
	attackMotion_.SetDuration(attack);
	recoveryMotion_.SetDuration(recovery);
	returnMotion_.SetDuration(returnTime);
}

void BossPoseSkill::SetFinishCondition(
	AttackStateType phase,
	PhaseFinishCondition condition)
{
	finishConditions_[phase] = condition;
}

bool BossPoseSkill::IsPhaseFinished(AttackStateType phase) const
{
	auto it = finishConditions_.find(phase);

	const auto condition = it != finishConditions_.end()
		? it->second
		: PhaseFinishCondition::MotionFinished;

	switch (condition)
	{
	case PhaseFinishCondition::MotionFinished:
		return GetPhaseMotion(phase).IsFinished();

	case PhaseFinishCondition::AnimationFinished:
		return boss_.GetAnimation().IsFinished();

	case PhaseFinishCondition::AnimationBlendFinished:
		return !boss_.GetAnimation().IsBlending();
	}

	return false;
}

const Motion& BossPoseSkill::GetPhaseMotion(AttackStateType phase) const
{
	switch (phase)
	{
	case AttackStateType::PreAction: return preActionMotion_;
	case AttackStateType::Aiming:    return aimingMotion_;
	case AttackStateType::Attack:    return attackMotion_;
	case AttackStateType::Recovery:  return recoveryMotion_;
	case AttackStateType::Return:    return returnMotion_;
	}

	return preActionMotion_;
}

Motion& BossPoseSkill::GetPhaseMotion(AttackStateType phase)
{
    return const_cast<Motion&>(
        static_cast<const BossPoseSkill&>(*this).GetPhaseMotion(phase));
}