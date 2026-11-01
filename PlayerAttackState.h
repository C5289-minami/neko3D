#pragma once
#include "PlayerBaseState.h"
#include "Collision.h"

class PlayerAttackState : public PlayerBaseState
{
public:
    PlayerAttackState(PlayerStateMachine& stateMachine, Player& player);
    void Enter() override;
    void Tick(float deltaTime) override;
    void Exit() override;
	void Draw() const;
	const Collision::Sphere& GetAttackSphere() const { return attackSphere_; }
	bool IsBallActive() const { return ballActive_; }
	bool TryRegisterHit();

private:
	bool ballActive_{};
	bool hasHitTarget_{};
	Collision::Sphere attackSphere_{};
	float elapsed_{};
};
