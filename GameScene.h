// =============================
// Scenes/Game/GameScene.h
// =============================
#pragma once
#include "Scene.h"
#include "GameStateMachine.h"
#include "ModelCollision.h"


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

	GameStateType GetCurrentGameState() const { return stateMachine_.GetCurrentType(); }
	bool UpdateStateCamera(float deltaTime, const std::string& requestedShot);
 void FocusTutorialCamera(float deltaTime, const Vec3& cameraPositionOffset);
	Camera::FreeCamera& GetFreeCamera() { return camera_; }
	Camera::TargetCamera& GetTargetCamera() { return targetCamera_; }
	Camera::CameraDirector& GetCameraDirector() { return cameraDirector_; }

private:

    ModelCollision tutorialCollision_;
    ModelToonRenderer modelToonRenderer_{};
    ShaderEffectController shaderEffects_{};
    void DrawScene(bool toonEnabled) const;
    void RenderDepth() const;

    std::string transformInitialStatus_;
    bool debugActorsPaused_{};

	// ƒJƒƒ‰
	Camera::FreeCamera camera_;
	Camera::TargetCamera targetCamera_;
	Camera::CameraDirector cameraDirector_{ camera_ }; // ƒJƒƒ‰‚Ì‘JˆÚ‚Æ‰‰o‚ğŠÇ—‚·‚é
	std::string activeCameraShot_{};

	GameStateMachine stateMachine_;

};
