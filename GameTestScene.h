
#pragma once

#include "Scene.h"
#include "ModelToonRenderer.h"
#include "ShaderEffectController.h"
#include "Camera.h"
#include "Collision.h"
#include "CameraDirector.h"

class Boss;
class Player;

class GameTestScene final : public Scene
{
public:
    explicit GameTestScene(GameContext* context) : Scene(context) {}

    void Init() override;
    void Update(float deltaTime) override;
    void Render() const override;
    void DrawImGui() override;
    void End() override;

	void UpdateTailWhipGimmick();

private:
    // Camera
    Camera::TargetCamera playerCamera_;

    Camera::CameraDirector cameraDirector_{ playerCamera_ };
#ifndef NDEBUG
    Camera::FreeCamera freeCamera_;
    bool wasFreeCameraToggleDown_{};
#endif

    // Shader
    ModelToonRenderer modelToonRenderer_{};
    ShaderEffectController shaderEffects_{};

    // 尻尾攻撃のデバッグ描画と本判定で共有するカプセル。
    Collision::Capsule tailWhipHitCapsule_{};

    // Debug
    bool rightMouseWasDown_{};
    float attackSphereDebugRemaining_{};
    bool debugActorsPaused_{};

    // Rendering
    void DrawScene(bool toonEnabled) const;
    void RenderDepth() const;
    void DrawAttackSphereDebug() const;

	bool wasTailWhipActive_{ false }; // 前フレームで尻尾攻撃が有効だったかどうか

   void UpdateCameraShake(Boss& boss, Player& player);
};
