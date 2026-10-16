#pragma once
#include "Scene.h"
#include "CharSettings.h"
#include "Camera.h" 
#include "ModelToonRenderer.h"
#include "ShaderEffectController.h"

#include "DrawableObject.h"
#include "Grid.h"
#include "Player.h"

// =============================
// キャラの作成、アニメーションの確認、
// モーションのテストなどを行うシーン
// サンドバックやBossで確認も
// =============================

class StatusTestScene final :  public Scene
{
public:
	explicit StatusTestScene(class GameContext* context)
		: Scene(context)
	{
	}
	void Init() override;
	void Update(float deltaTime) override;
    void Render() const override;
	void DrawImGui() override;
private:
	CharSettingsManager charSettings_;

	Camera::HomingCamera camera_;
	Camera::FreeCamera freeCamera_;
	Camera::Camera* activeCamera_{ nullptr };
		
	// モデルのトゥーン描画を管理する
	ModelToonRenderer modelToonRenderer_{};
	// 共通のスクリーン空間アウトライン描画
	ShaderEffectController shaderEffects_{};
	void DrawScene(bool toonEnabled) const;
	void RenderDepth() const;

	Player player_{};
	ModelObject testModel_{};
	Grid grid_{};
};

