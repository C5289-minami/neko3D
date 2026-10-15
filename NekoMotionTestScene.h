#pragma once
#include "Scene.h"
#include "Player.h"
#include "Room.h"
#include "Camera.h"
#include "Grid.h"
#include "ModelToonRenderer.h"
#include "ShaderEffectController.h"

class NekoMotionTestScene final: public Scene
{
public:
	explicit NekoMotionTestScene(class GameContext* context) : Scene(context) {}
	void Init() override;
	void Update(float deltaTime) override;
	void Render() const override;
private:
	Player player_;
	Room room_;
	Camera::FreeCamera camera_;
	Grid grid_;


	// モデルのトゥーン描画を管理する
	ModelToonRenderer modelToonRenderer_{};
	// 共通のスクリーン空間アウトライン描画
	ShaderEffectController shaderEffects_{};
	void DrawScene(bool toonEnabled) const;
	void RenderDepth() const;
};

