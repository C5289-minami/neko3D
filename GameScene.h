// =============================
// Scenes/Game/GameScene.h
// =============================
#pragma once
#include "Scene.h"
#include "GameContext.h"
#include "ModelToonRenderer.h"
#include "ShaderEffectController.h"
#include "Camera.h"
#include "TransformSettings.h"

class GameScene final : public Scene
{
public:
    explicit GameScene(GameContext* context) : Scene(context) {}
    void Init() override;
    void Update(float deltaTime) override;
    void Render() const override;
    void End() override;

	void SetActiveCamera(Camera::Camera* camera) { activeCamera_ = camera; }
    DebugSceneControls GetDebugControls();


private:
	Camera::Camera* activeCamera_{ nullptr };
    Camera::HomingCamera playerCamera_;
#ifndef NDEBUG
    Camera::FreeCamera freeCamera_;
    bool wasFreeCameraToggleDown_{};
#endif

    // モデルのトゥーン描画を管理する
    ModelToonRenderer modelToonRenderer_{};
    // 共通のスクリーン空間アウトライン描画
    ShaderEffectController shaderEffects_{};
    void DrawScene(bool toonEnabled) const;
    void RenderDepth() const;

    std::string transformInitialStatus_;
    bool debugActorsPaused_{};
};
