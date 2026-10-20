// =============================
// Scenes/Game/GameScene.h
// =============================
#pragma once
#include "Scene.h"
#include "ModelToonRenderer.h"
#include "ShaderEffectController.h"
// camera
#include "Camera.h"
#include "CameraDirector.h"

class GameScene final : public Scene
{
public:
	explicit GameScene(GameContext* context) : Scene(context) {}
	void Init() override;
	void Update(float deltaTime) override;
	void Render() const override;

private:

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

};
