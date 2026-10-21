#pragma once
#include <string>
#include "DxPlus/Vector2.h"
#include "Vector3.h"
#include "Collision.h"
#include "ModelType.h"

class ActorBase;

struct ImageObject
{
	std::wstring imageKey;
	DxPlus::Vec2 position{};
	DxPlus::Vec2 scale{ 1.0f, 1.0f };
	DxPlus::Vec2 center{ 0.5f, 0.5f };
	float rotation{};

	DxPlus::Vec2 GetSize() const;
	void Draw(DxPlus::Vec2 masterPos = { 0.0f,0.0f }) const;
};

struct ModelObject
{
	

	bool collisionHighlighted{ false };
	const ActorBase* owner_{ nullptr };

	void Init();
	void Draw(Vec3 masterPos = { 0.0f, 0.0f, 0.0f }) const;
	// JSONファイルから読み込み、オブジェクトを初期化する
	bool Load(const std::string& filePath = "./Data/Config/model_objects.json");
	bool Save(const std::string& filePath = "./Data/Config/model_objects.json") const;

	// ゲッターセッター
	void ApplyTransform(Vec3 masterPos = { 0.0f, 0.0f, 0.0f }) const;
	void ApplyCollisionColor() const;
	//センターやスケールを考慮して、楕円体の当たり判定を返す　回転は無視
	Collision::Ellipsoid GetHitEllipsoid() const;
	Vec3 GetSize() const { return GetHitEllipsoid().radii * 2.0f; }
    void SetOwner(const ActorBase& owner) { owner_ = &owner; }
	ModelObject WithParentTransform(const Vec3& parentPosition, const Vec3& parentRotation) const;
	void SetAlpha(float value) { alpha = (std::max)(0.0f, (std::min)(1.0f, value)); }
	void GetAlpha(float& outValue) const { outValue = alpha; }
	float GetAlpha() const { return alpha; }
	void FadeTo(const class ActorBase& player, float speed, float deltaTime);
    const std::wstring& GetModelKey() const { return modelKey; }
	void SetModelKey(const std::wstring& value) { modelKey = value; }
	Vec3 GetPosition() const { 
		// 取得前に親子行列を適応
		ApplyTransform();
		return position;
	}
	void SetPosition(const Vec3& value) { position = value; }
	Vec3 GetScale() const { return scale; }
	void SetScale(const Vec3& value) { scale = value; }
	Vec3 GetCenter() const { return center; }
	void SetCenter(const Vec3& value) { center = value; }
	Vec3 GetRotation() const { 
		// 取得前に親子行列を適応
		ApplyTransform();
		return rotation;
	}
	void SetRotation(const Vec3& value) { rotation = value; }
	ModelType GetModelType() const { return modelType_; }
	void SetModelType(ModelType type) { modelType_ = type; }


	

private:
	std::wstring modelKey;
	Vec3 position{};
	Vec3 scale{ 1.0f, 1.0f, 1.0f };
	Vec3 center{ 0.0f, 0.0f, 0.0f }; // スケールと同期はできないが回転の中心をずらすために使用する
	Vec3 rotation{};
	float alpha{ 1.0f };
	ModelType modelType_ = ModelType::Model;
};
