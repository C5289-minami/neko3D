#pragma once

#include <string>

#include "Boss.h"
#include "Scene.h"
#include "GameContext.h"
#include "DrawableObject.h"
#include "Vector3.h"
#include "Stage.h"
#include "Player.h"
#include "ModelToonRenderer.h"
#include "ShaderEffectController.h"
#include "Camera.h"

class ShaderTest final : public Scene
{
public:
    explicit ShaderTest(class GameContext* context)
        : Scene(context)
    {
    }

    void Init() override;
    void Update(float deltaTime) override;
    void Render() const override;
	void DrawImGui() override;

private:
  
    // Debug
    Camera::FreeCamera camera_{};
    bool debugActorsPaused_{};
    std::string transformInitialStatus_;


    // モデルのトゥーン描画を管理する
    ModelToonRenderer modelToonRenderer_{};
    // 共通のスクリーン空間アウトライン描画
    ShaderEffectController shaderEffects_{};
    void DrawScene(bool toonEnabled) const;
    void RenderDepth() const;

};
