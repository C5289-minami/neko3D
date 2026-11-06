// =============================
// Scenes/Game/GameScene.h
// =============================
#pragma once
#include "Scene.h"
#include "GameStateMachine.h"
#include "ModelCollision.h"
#include "Vector3.h"
#include "ObjectTransform.h"
#include <optional>

// shader effects
#include "ModelToonRenderer.h"
#include "ShaderEffectController.h"

// camera
#include "Camera.h"
#include "CameraDirector.h"
#include "Motion.h"
#include <string>

// Tutorial
#include "TutorialChecklist.h"
#include "TutorialType.h"

class GameScene final : public Scene
{
public:
	explicit GameScene(GameContext* context);
	void Init() override;
  void Update(float deltaTime) override;
    void UpdateDuringFade(float deltaTime) override;
    void End() override;
	void Render() const override;
	void DrawImGui() override;
	void DrawPlayerShadow()const;
	void ResetForTitle();
	void SaveBattleCheckpoint();
	bool RestoreBattleCheckpoint();

	GameStateType GetCurrentGameState() const { return stateMachine_.GetCurrentType(); }
	bool UpdateStateCamera(float deltaTime, const std::string& requestedShot);
	Camera::FreeCamera& GetFreeCamera() { return camera_; }
	Camera::TargetCamera& GetTargetCamera() { return playerTargetCamera_; }
    Camera::TargetCamera& GetBossDieCamera() { return bossDieCamera_; }
    Camera::TargetCamera& GetBossDownCamera() { return bossDownCamera_; }
	Camera::CameraDirector& GetCameraDirector() { return cameraDirector_; }

    // チュートリアルの球体設定
    struct TutorialCollisionSettings
    {
        // Player
        Vec3 playerOffset{ 0.0f, 20.0f, 0.0f };
        float playerRadius = 20.0f;

        // Stage
        Vec3 stageOffset{ 0.0f, 0.0f, 0.0f };
        float stageRadius = 1200.0f;

        // カメラ追従を開始する球体
        float climbRadius = 2500.0f;

        // デバッグ描画
        bool drawSpheres = true;
    };

	// チュートリアル

    TutorialCollisionSettings& GetTutorialCollisionSettings()
    {
        return tutorialCollisionSettings_;
    }
    const TutorialCollisionSettings& GetTutorialCollisionSettings() const
    {
        return tutorialCollisionSettings_;
    }
    TutorialChecklist& GetTutorialChecklist()
    {
        return tutorialChecklist_;
    }
    TutorialTaskManager& GetTutorialTaskManager()
    {
        return taskManager_;
	}

    // 影
	void SetDrawPlayerShadow(bool draw) { drawPlayerShadow_ = draw; }
    void SetBattleEventActive(bool active) { battleEventActive_ = active; }
    bool IsBattleEventActive() const { return battleEventActive_; }
    void StartBattleEventBarsIn()
    {
        battleLetterboxMotion_.Start(0.0f, 1.0f, 0.5f, Motion::Easing::EaseOut);
    }
    void StartBattleEventBarsOut()
    {
        battleLetterboxMotion_.Start(1.0f, 0.0f, 0.5f, Motion::Easing::EaseIn);
    }


private:

    ModelCollision tutorialCollision_;
    ModelToonRenderer modelToonRenderer_{};
    ShaderEffectController shaderEffects_{};
    void DrawScene(bool toonEnabled) const;
    void RenderDepth() const;

    std::string transformInitialStatus_;

	// カメラ
	Camera::FreeCamera camera_;
    Camera::TargetCamera bossDieCamera_;
    Camera::TargetCamera bossDownCamera_;
	Camera::TargetCamera playerTargetCamera_;
	Camera::CameraDirector cameraDirector_{ camera_ }; // カメラの遷移と演出を管理する
	std::string activeCameraShot_{};

	GameStateMachine stateMachine_;

	struct BattleCheckpoint
	{
		ObjectTransform playerTransform;
		ObjectTransform bossTransform;
		Vec3 bossPosition;
		Vec3 bossRotation;
		Camera::CameraPose cameraPose;
		Camera::TargetCamera targetCamera;
	};
	std::optional<BattleCheckpoint> battleCheckpoint_;

	TutorialCollisionSettings tutorialCollisionSettings_{}; // チュートリアルの球体設定
    void DrawTutorialCollisionSpheres() const;
	TutorialChecklist tutorialChecklist_; // チュートリアルのチェックリストUI
	TutorialTaskManager taskManager_; // チュートリアルの進捗管理

	// Playerの影を描画する
	bool drawPlayerShadow_{ false };
    bool battleEventActive_{ false };
    Motion battleLetterboxMotion_{};

};
