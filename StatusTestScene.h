#pragma once
#include "Scene.h"
#include "CharSettings.h"
#include "Camera.h" 

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
private:
	CharSettingsManager charSettings_;
	Camera::HomingCamera camera_;
};

