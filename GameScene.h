// =============================
// Scenes/Game/GameScene.h
// =============================
#pragma once
#include "Scene.h"
#include "GameStateMachine.h"
#include "ModelCollision.h"
#include "Vector3.h"

// shader effects
#include "ModelToonRenderer.h"
#include "ShaderEffectController.h"

// camera
#include "Camera.h"
#include "CameraDirector.h"
#include <string>

class GameScene final : public Scene
{
public:
	explicit GameScene(GameContext* context);
	void Init() override;
	void Update(float deltaTime) override;
    void End() override;
	void Render() const override;
	void DrawImGui() override;

	GameStateType GetCurrentGameState() const { return stateMachine_.GetCurrentType(); }
	bool UpdateStateCamera(float deltaTime, const std::string& requestedShot);
	Camera::FreeCamera& GetFreeCamera() { return camera_; }
	Camera::TargetCamera& GetTargetCamera() { return targetCamera_; }
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
        float climbRadius = 2000.0f;

        // デバッグ描画
        bool drawSpheres = true;
    };

    TutorialCollisionSettings& GetTutorialCollisionSettings()
    {
        return tutorialCollisionSettings_;
    }

    const TutorialCollisionSettings& GetTutorialCollisionSettings() const
    {
        return tutorialCollisionSettings_;
    }
private:

    ModelCollision tutorialCollision_;
    ModelToonRenderer modelToonRenderer_{};
    ShaderEffectController shaderEffects_{};
    void DrawScene(bool toonEnabled) const;
    void RenderDepth() const;

    std::string transformInitialStatus_;
    bool debugActorsPaused_{};

	// カメラ
	Camera::FreeCamera camera_;
	Camera::TargetCamera targetCamera_;
	Camera::CameraDirector cameraDirector_{ camera_ }; // カメラの遷移と演出を管理する
	std::string activeCameraShot_{};

	GameStateMachine stateMachine_;

    TutorialCollisionSettings tutorialCollisionSettings_{};
    void DrawTutorialCollisionSpheres() const;
};
