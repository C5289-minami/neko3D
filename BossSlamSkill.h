#pragma once

#include "BossAttackType.h"
#include "BossPoseSkill.h"

class BossSlamSkill final : public BossPoseSkill
{
public:
	static constexpr BossAttackType Type = BossAttackType::Slam;

	explicit BossSlamSkill(Boss& boss) : BossPoseSkill(boss)
	{
		SetFinishCondition(AttackStateType::PreAction, PhaseFinishCondition::AnimationFinished);
		SetFinishCondition(AttackStateType::Aiming, PhaseFinishCondition::MotionFinished);
		SetFinishCondition(AttackStateType::Attack, PhaseFinishCondition::AnimationFinished);
		SetFinishCondition(AttackStateType::Return, PhaseFinishCondition::AnimationFinished);

		//GetPhaseMotion(AttackStateType::Aiming).SetDuration(3.0f);// 狙う時間を指定
	}

private:
	void EnterPhase(AttackStateType phase) override
	{
		switch (phase)
		{
		case AttackStateType::PreAction:
			// AnimTypeをSlamに設定し、アニメーションを再生する
			boss_.SetAnimation(BossAnimType::Slam, false);
			boss_.GetAnimation().PlayTo(0.0f, Const::ANIM_FPS, 3.0f);
			break;
		case AttackStateType::Aiming:
			// アニメーションの再生位置を設定する
			boss_.GetAnimation().PlayTo(0.0f);
			break;
		case AttackStateType::Attack:
			boss_.GetAnimation().PlayTo(8.0f);
			break;
		case AttackStateType::Recovery:
		case AttackStateType::Return:
			// アニメーションの再生を開始する
			boss_.GetAnimation().Start();
			break;
		}
	}
	void ApplyPhase(AttackStateType phase, float progress) override
	{
		auto& animation = boss_.GetAnimation();

		switch (phase)
		{
		case AttackStateType::PreAction:
			break;

		case AttackStateType::Aiming:
			break;

		case AttackStateType::Attack:
			break;

		case AttackStateType::Recovery:
			break;

		case AttackStateType::Return:
			break;
		}
	}

	void EndPhase(AttackStateType phase) override
	{
		switch (phase)
		{
		case AttackStateType::PreAction:
			break;
		case AttackStateType::Aiming:
			break;
		case AttackStateType::Attack:
			break;
		case AttackStateType::Recovery:
			break;
		case AttackStateType::Return:
			break;
		}
	}
};