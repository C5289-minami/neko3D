#pragma once
#include "Scene.h"
#include "GameContext.h"
#include "ModelToonRenderer.h"
#include "ShaderEffectController.h"
#include "Camera.h"

class GameTestScene final : public Scene
{
public:
    explicit GameTestScene(GameContext* context) : Scene(context) {}
    void Init() override;
    void Update(float deltaTime) override;
    void Render() const override;
    void DrawImGui() override;
    void End() override;

    void SetActiveCamera(Camera::Camera* camera) { activeCamera_ = camera; }

private:
    Camera::Camera* activeCamera_{ nullptr };
    Camera::TargetCamera playerCamera_;
#ifndef NDEBUG
    Camera::FreeCamera freeCamera_;
    bool wasFreeCameraToggleDown_{};
#endif

    ModelToonRenderer modelToonRenderer_{};
    ShaderEffectController shaderEffects_{};
    void DrawScene(bool toonEnabled) const;
    void RenderDepth() const;

    std::string transformInitialStatus_;
    bool debugActorsPaused_{};
};

