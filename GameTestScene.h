
#pragma once

#include "Scene.h"
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

private:
    // Camera
    Camera::TargetCamera playerCamera_;

#ifndef NDEBUG
    Camera::FreeCamera freeCamera_;
    Camera::Camera* activeCamera_{ nullptr };
    bool wasFreeCameraToggleDown_{};
#endif

    // Shader
    ModelToonRenderer modelToonRenderer_{};
    ShaderEffectController shaderEffects_{};

    // Debug
    bool rightMouseWasDown_{};
    float attackSphereDebugRemaining_{};
    bool debugActorsPaused_{};

    // Rendering
    void DrawScene(bool toonEnabled) const;
    void RenderDepth() const;
    void DrawAttackSphereDebug() const;
};
