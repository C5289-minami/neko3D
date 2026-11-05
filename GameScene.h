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

// Tutorial
#include "TutorialChecklist.h"
#include "TutorialType.h"

class GameScene final : public Scene
{
public:
	explicit GameScene(GameContext* context);
	void Init() override;
	void Update(float deltaTime) override;
    void End() override;
	void Render() const override;
	void DrawImGui() override;
	void DrawPlayerShadow()const;

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


private:

    ModelCollision tutorialCollision_;
    ModelToonRenderer modelToonRenderer_{};
    ShaderEffectController shaderEffects_{};
    void DrawScene(bool toonEnabled) const;
    void RenderDepth() const;

    std::string transformInitialStatus_;

	// カメラ
	Camera::FreeCamera camera_;
	Camera::TargetCamera targetCamera_;
	Camera::CameraDirector cameraDirector_{ camera_ }; // カメラの遷移と演出を管理する
	std::string activeCameraShot_{};

	GameStateMachine stateMachine_;

	TutorialCollisionSettings tutorialCollisionSettings_{}; // チュートリアルの球体設定
    void DrawTutorialCollisionSpheres() const;
	TutorialChecklist tutorialChecklist_; // チュートリアルのチェックリストUI
	TutorialTaskManager taskManager_; // チュートリアルの進捗管理

	// Playerの影を描画する
	bool drawPlayerShadow_{ false };
	// ボス本体のカプセルに前フレームから接触しているか。
	bool wasTouchingBossBody_{ false };
    // ダブルジャンプで触れ始めたときの復帰を一度だけ行う。
    bool wasDoubleJumpContact_{};

};
