#pragma once
#include "Scene.h"
#include "Grid.h"
#include "ModelToonRenderer.h"
#include "ShaderEffectController.h"

#include "Camera.h"
#include "CameraDirector.h"

class RoomTestScene final: public Scene
{
public:
	explicit RoomTestScene(class GameContext* context) : Scene(context) {}
	void Init() override;
	void Update(float deltaTime) override;
	void Render() const override;
private:
	// モデルのトゥーン描画を管理する
	ModelToonRenderer modelToonRenderer_{};
	// 共通のスクリーン空間アウトライン描画
	ShaderEffectController shaderEffects_{};
	void DrawScene(bool toonEnabled) const;
	void RenderDepth() const;


	// メンバー変数
	Camera::FreeCamera camera_;
	Camera::CameraDirector cameraDirector_{ camera_ };

	bool wasCameraMoveKeyDown_{};
	bool wasCameraCloseUpKeyDown_{};
	bool wasCameraFollowKeyDown_{};

};

