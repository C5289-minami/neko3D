#pragma once
#include "Scene.h"
#include "Camera.h"
#include "Grid.h"
#include "ModelToonRenderer.h"
#include "ShaderEffectController.h"

class RoomTestScene final: public Scene
{
public:
	explicit RoomTestScene(class GameContext* context) : Scene(context) {}
	void Init() override;
	void Update(float deltaTime) override;
	void Render() const override;
#ifndef NDEBUG
	void SetModelEditTargets() override;
#endif
private:
	Camera::FreeCamera camera_;


	// モデルのトゥーン描画を管理する
	ModelToonRenderer modelToonRenderer_{};
	// 共通のスクリーン空間アウトライン描画
	ShaderEffectController shaderEffects_{};
	void DrawScene(bool toonEnabled) const;
	void RenderDepth() const;
};

