#pragma once
#include "GameBaseState.h"
#include "Vector3.h"
#include "CameraPose.h" 
#include "TutorialType.h"
#include "TutorialTaskManager.h"

class GameSceneTutorialState : public GameBaseState
{
public:
	GameSceneTutorialState(GameStateMachine& gameStateMachine, GameScene& gameScene);

	void Enter() override;
	void Tick(float deltaTime) override;
	void Exit() override;

private:
	void FocusTutorialCamera(float deltaTime);

	bool targetCameraSwitched_{ false };
	bool tutorialCameraMoved_{ false };

	float cameraPlayerHeightOffset = 300.0f; // プレイヤーの頭上にカメラを置く
	float cameraDistance = 750.0f; // プレイヤーからの距離
	float cameraFov = 50.0f;  // 画角
	float cameraPitch = -0.2f;   // 上下の角度
	float cameraFollowSpeed = 180.0f; // 普段のカメラ移動スピード（ゆっくり）
	float cameraMaxLag = 150.0f; // これ以上離れたら急いで追いつく距離
	float cameraTurnSpeed = 6.0f;   // カメラがプレイヤーを向く速さ（遅れないよう少し上げる）
	
	Camera::CameraPose GetTutorialCameraPose();

	enum class TutorialCameraState
	{
		LookPlayer,   // 固定カメラからPlayerを見る
		EnterMove,    // 追従開始位置まで移動中
		FollowPlayer  // Playerを追従
	};

	TutorialCameraState cameraState_ =
		TutorialCameraState::LookPlayer;



};