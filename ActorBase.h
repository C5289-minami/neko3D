#pragma once
#include "Vector3.h"


class ActorBase
{
public:
	ActorBase() = default;
	virtual ~ActorBase() = default;
	virtual void Init() = 0;
	virtual void Reset() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Draw() const = 0;

	// ゲッターセッター
	const Vec3& GetPosition() const { return position_; }
	void SetPosition(const Vec3& position) { position_ = position; }
	float GetYaw() const { return yaw_; }
	void SetYaw(float yaw) { yaw_ = yaw; }
	float GetPitch() const { return pitch_; }
	void SetPitch(float pitch) { pitch_ = pitch; }
protected:
	Vec3 position_{ 0.0f, 0.0f, 0.0f };
	float yaw_{ 0.0f };
	float pitch_{ 0.0f };
};
