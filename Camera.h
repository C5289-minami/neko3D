#pragma once
#include "Vector3.h"
#include "DxLib.h"
#include "DxConv.h"
#include "DxPlus/DxPlus.h"
#include <algorithm>


class Camera
{
public:
	Camera() = default;
	virtual ~Camera() = default;

	virtual void Reset()
	{
		position_ = Vec3(0.0f, 0.0f, 0.0f);
		yaw_ = 0.0f;
		pitch_ = 0.0f;
	}

	virtual void ApplyCamera()const	{

		const float cosPitch = std::cos(pitch_);
		const Vec3 forward{
			std::sin(yaw_) * cosPitch,
			std::sin(pitch_),
			std::cos(yaw_) * cosPitch
		};
		const Vec3 target = position_ + forward;
		DxLib::SetCameraPositionAndTargetAndUpVec(
			DxConv::ToVECTOR(position_),
			DxConv::ToVECTOR(target),
			VGet(0.0f, 1.0f, 0.0f)
		);
	}

	void SetYaw(float yaw) { yaw_ = yaw; }
	void SetPitch(float pitch) { pitch_ = pitch; }
	const Vec3& GetPosition() const { return position_; }
	void SetPosition(const Vec3& position) { position_ = position; }
	float GetYaw() const { return yaw_; }
	float GetPitch() const { return pitch_; }

protected:
	Vec3 position_{};
	float yaw_{};
	float pitch_{};
};

class FreeCamera : public Camera
{
public:
	FreeCamera() = default;
	~FreeCamera() = default;


	void InputUpdate(float deltaTime,
		float moveSpeed = 500.0f,
		float rotateSpeed = 0.2f * DxPlus::Deg2Rad)
	{
		Vec3 forward{
			std::cos(pitch_) * std::sin(yaw_),
			std::sin(pitch_),
			std::cos(pitch_) * std::cos(yaw_)
		};

		forward = forward.Normalized();
		Vec3 right{
			std::cos(yaw_),
			0.0f,
			-std::sin(yaw_)
		};
		right = right.Normalized();

		Vec3 move{};

		if (DxLib::CheckHitKey(KEY_INPUT_W)) move += forward * moveSpeed * deltaTime;
		if (DxLib::CheckHitKey(KEY_INPUT_S)) move -= forward * moveSpeed * deltaTime;
		if (DxLib::CheckHitKey(KEY_INPUT_A)) move -= right * moveSpeed * deltaTime;
		if (DxLib::CheckHitKey(KEY_INPUT_D)) move += right * moveSpeed * deltaTime;
		if (DxLib::CheckHitKey(KEY_INPUT_Q)) move.y -= moveSpeed * deltaTime;
		if (DxLib::CheckHitKey(KEY_INPUT_E)) move.y += moveSpeed * deltaTime;

		if (DxLib::CheckHitKey(KEY_INPUT_LSHIFT))
			move *= 2.0f; // Shiftキーで移動速度を倍にする
		position_ += move;

		// マウス入力による回転
		int mouseX, mouseY;
		DxLib::GetMousePoint(&mouseX, &mouseY);

		const auto mouseDelta = DxPlus::Input::GetMouseDelta();
		yaw_ += mouseDelta.x * rotateSpeed;
		pitch_ -= mouseDelta.y * rotateSpeed;

		pitch_ = std::clamp(pitch_, -DxPlus::Deg2Rad * 89.0f, DxPlus::Deg2Rad * 89.0f);


		ApplyCamera();
	}

private:
};

class HomingCamera : public Camera
{
public:
	HomingCamera() = default;
	~HomingCamera() = default;

	void SetTarget(const Vec3& target) { target_ = target; }
	const Vec3& GetTarget() const { return target_; }
private:
	Vec3 target_{};
};
