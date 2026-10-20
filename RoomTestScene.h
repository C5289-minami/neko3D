#pragma once
#include "Scene.h"
#include "Grid.h"
#include "ModelToonRenderer.h"
#include "ShaderEffectController.h"

#include "Camera.h"
#include "CameraDirector.h"

#include <string>

class RoomTestScene final: public Scene
{
public:
	explicit RoomTestScene(class GameContext* context) : Scene(context) {}
	void Init() override;
	void Update(float deltaTime) override;
	void Render() const override;
	void DrawImGui() override;


private:
	// モデルのトゥーン描画を管理する
	ModelToonRenderer modelToonRenderer_{};
	// 共通のスクリーン空間アウトライン描画
	ShaderEffectController shaderEffects_{};
	void DrawScene(bool toonEnabled) const;
	void RenderDepth() const;


	Camera::FreeCamera camera_;
	Camera::TargetCamera targetCamera_;
	Camera::CameraDirector cameraDirector_{ camera_ }; // カメラの遷移と演出を管理する
	char savedCameraName_[128]{ "Camera" };
	std::string cameraStatus_;

};

