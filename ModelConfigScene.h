#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "Camera.h"
#include "DrawableObject.h"
#include "Scene.h"

#include <string>
#include "ModelToonRenderer.h"
#include "ShaderEffectController.h"

class ActorBase;

class ModelConfigScene final : public Scene
{
public:
    explicit ModelConfigScene(GameContext* context) : Scene(context) {}
    void Init() override;
    void Update(float deltaTime) override;
    void Render() const override;
    void DrawImGui() override;

private:
	Camera::FreeCamera camera_; // カメラ
	ModelObject* selected_{};   // 選択中のオブジェクト
    ActorBase* selectedActor_{};
	std::string status_;        // ImGuiのステータス表示用

    // モデルのトゥーン描画を管理する
    ModelToonRenderer modelToonRenderer_{};
    // 共通のスクリーン空間アウトライン描画
    ShaderEffectController shaderEffects_{};
    void DrawScene(bool toonEnabled) const;
    void RenderDepth() const;
};
