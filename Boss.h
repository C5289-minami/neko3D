#pragma once
#pragma once

#include "AnimationDraw.h"
#include "BossStateMachine.h"
#include "DrawableObject.h"
#include "IAttackSkill.h"
#include "Vector3.h"

class Boss
{
public:
    Boss();

    void Init();
    void Reset();
    void Update(float deltaTime);
    void Draw() const;

    void SetMoveDirection(const Vec3& moveDirection, float speed);
    void StopMove();
    bool UseSkill(IAttackSkill& skill);
    IAttackSkill* GetActiveSkill() const { return activeSkill_; }
    void ClearActiveSkill() { activeSkill_ = nullptr; }
    const Vec3& GetPosition() const { return model_.position; }
    void SetPosition(const Vec3& position) { model_.position = position; }
    BossStateType GetCurrentState() const { return stateMachine_.GetCurrentType(); }
	ModelObject& GetModelObject() { return model_; }

private:
    ModelObject model_{};
    Vec3 velocity_{};
    IAttackSkill* activeSkill_{ nullptr };
    mutable AnimationDraw animationDraw_;
    BossStateMachine stateMachine_;
};
