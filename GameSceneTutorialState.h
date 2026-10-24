#pragma once
#include "GameBaseState.h"
#include "Vector3.h"

class GameSceneTutorialState : public GameBaseState
{
public:
	GameSceneTutorialState(GameStateMachine& gameStateMachine, GameScene& gameScene);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;

private:
	Vec3 cameraPositionOffset_{};
	bool targetCameraSwitched_{ false };
	bool tutorialCameraMoved_{ false };
};