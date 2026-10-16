#pragma once

#include "Vector3.h"
#include "DxLib.h"
#include "DxConv.h"
#include "DxPlus/DxPlus.h"

#include <algorithm>
#include <cmath>

#ifndef NDEBUG
#include "imgui.h"
#endif

namespace Camera
{
	class Camera
	{
	public:
		virtual ~Camera() = default;

		virtual void Reset()
		{
			position_ = {};
			yaw_ = 0.0f;
			pitch_ = 0.0f;
		}
		virtual void Update(float deltaTime) {}

		virtual void ApplyCamera() const
		{
			const float cosPitch = std::cos(pitch_);
			const Vec3 forward{
				std::sin(yaw_) * cosPitch,
				std::sin(pitch_),
				std::cos(yaw_) * cosPitch
			};
			ApplyView(position_ + forward);
		}

		void SetYaw(float yaw) { yaw_ = yaw; }
		void SetPitch(float pitch) { pitch_ = pitch; }
		const Vec3& GetPosition() const { return position_; }
		void SetPosition(const Vec3& position) { position_ = position; }
		float GetYaw() const { return yaw_; }
		float GetPitch() const { return pitch_; }

	protected:
		void ApplyView(const Vec3& target) const
		{
			DxLib::SetCameraPositionAndTargetAndUpVec(
				DxConv::ToVECTOR(position_),
				DxConv::ToVECTOR(target),
				VGet(0.0f, 1.0f, 0.0f));
		}

		Vec3 position_{};
		float yaw_{};
		float pitch_{};
	};

	class FreeCamera : public Camera
	{
 public:
		void Reset() override
		{
			Camera::Reset();
			homeEye_ = {};
			homeTarget_ = {};
			active_ = false;
			wasToggleDown_ = false;
			wasResetDown_ = false;
			wasFocusDown_ = false;
		}

		void Initialize(const Vec3& eye, const Vec3& target)
		{
			position_ = eye;
			const Vec3 direction = (target - eye).Normalized();
			pitch_ = std::asin(std::clamp(direction.y, -1.0f, 1.0f));
            yaw_ = std::atan2(direction.x, direction.z);
			homeEye_ = eye;
			homeTarget_ = target;
			active_ = false;
		}

       void Update(float deltaTime) override
		{
			UpdateFreeFlight(deltaTime, false, false);
			ApplyCamera();
		}

		void InputUpdate(float deltaTime) { Update(deltaTime); }

		void BeginSceneView()
		{
			if (active_) return;
			homeEye_ = position_;
			homeTarget_ = GetForwardTarget();
			active_ = true;
		}

        void ResetView()
		{
			const bool wasActive = active_;
			Initialize(homeEye_, homeTarget_);
			active_ = wasActive;
		}

		void FocusAt(const Vec3& point, float distance = 350.0f)
		{
			position_ = point - GetForward() * distance;
		}

		bool IsSceneViewActive() const { return active_; }
		const Vec3& GetEye() const { return position_; }
		Vec3 GetTarget() const { return GetForwardTarget(); }

		void UpdateSceneView(float deltaTime, const Vec3& focusPoint)
		{
#ifndef NDEBUG
			const bool keyboardCaptured = ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureKeyboard;
			const bool mouseCaptured = ImGui::GetCurrentContext() && ImGui::GetIO().WantCaptureMouse;
			const bool toggleDown = DxLib::CheckHitKey(KEY_INPUT_LALT) != 0 &&
				DxLib::CheckHitKey(KEY_INPUT_RETURN) != 0;
			if (toggleDown && !wasToggleDown_)
			{
				if (active_)
				{
					active_ = false;
					RestoreHomeView();
					SetMouseDispFlag(TRUE);
				}
				else
				{
					BeginSceneView();
					SetMouseDispFlag(FALSE);
					wasToggleDown_ = toggleDown;
					return;
				}
			}
			wasToggleDown_ = toggleDown;
			if (!active_) return;

			const bool resetDown = !keyboardCaptured && DxLib::CheckHitKey(KEY_INPUT_R) != 0;
			if (resetDown && !wasResetDown_) ResetView();
			wasResetDown_ = resetDown;
			const bool focusDown = !keyboardCaptured && DxLib::CheckHitKey(KEY_INPUT_F) != 0;
			if (focusDown && !wasFocusDown_) FocusAt(focusPoint);
			wasFocusDown_ = focusDown;

			UpdateFreeFlight(deltaTime, keyboardCaptured, mouseCaptured);
#else
			(void)deltaTime;
			(void)focusPoint;
#endif
		}

		void SetMoveSpeed(float speed) { moveSpeed_ = speed; }
		void SetRotateSpeed(float speed) { rotateSpeed_ = speed; }
		float GetMoveSpeed() const { return moveSpeed_; }
		float GetRotateSpeed() const { return rotateSpeed_; }

    private:
		Vec3 GetForward() const
		{
			const float cosPitch = std::cos(pitch_);
			return { std::sin(yaw_) * cosPitch, std::sin(pitch_), std::cos(yaw_) * cosPitch };
		}

		Vec3 GetForwardTarget() const { return position_ + GetForward(); }

		void RestoreHomeView()
		{
			position_ = homeEye_;
			const Vec3 direction = (homeTarget_ - homeEye_).Normalized();
			pitch_ = std::asin(std::clamp(direction.y, -1.0f, 1.0f));
			yaw_ = std::atan2(direction.x, direction.z);
		}

		void UpdateFreeFlight(float deltaTime, bool keyboardCaptured, bool mouseCaptured)
		{
			const Vec3 forward = GetForward();
			const Vec3 right{ std::cos(yaw_), 0.0f, -std::sin(yaw_) };
			Vec3 movement{};
			if (!keyboardCaptured && DxLib::CheckHitKey(KEY_INPUT_W)) movement += forward;
			if (!keyboardCaptured && DxLib::CheckHitKey(KEY_INPUT_S)) movement -= forward;
			if (!keyboardCaptured && DxLib::CheckHitKey(KEY_INPUT_A)) movement -= right;
			if (!keyboardCaptured && DxLib::CheckHitKey(KEY_INPUT_D)) movement += right;
			if (!keyboardCaptured && DxLib::CheckHitKey(KEY_INPUT_Q)) movement.y -= 1.0f;
			if (!keyboardCaptured && DxLib::CheckHitKey(KEY_INPUT_E)) movement.y += 1.0f;
			if (movement.LengthSq() > 0.0f)
			{
				const float speed = moveSpeed_ *
					(DxLib::CheckHitKey(KEY_INPUT_LSHIFT) ? 2.0f : 1.0f);
				position_ += movement.Normalized() * (speed * deltaTime);
			}

			const DxPlus::Vec2Int mouseDelta = mouseCaptured ? DxPlus::Vec2Int{} : DxPlus::Input::GetMouseDelta();
			yaw_ += mouseDelta.x * rotateSpeed_;
			pitch_ = std::clamp(pitch_ - mouseDelta.y * rotateSpeed_,
				-DxPlus::Deg2Rad * 89.0f, DxPlus::Deg2Rad * 89.0f);
			SetMousePoint(DxPlus::CLIENT_WIDTH / 2, DxPlus::CLIENT_HEIGHT / 2);
		}

		float moveSpeed_{ 500.0f };
		float rotateSpeed_{ 0.2f * DxPlus::Deg2Rad };
      bool active_{};
		bool wasToggleDown_{};
		bool wasResetDown_{};
		bool wasFocusDown_{};
		Vec3 homeEye_{};
		Vec3 homeTarget_{};
	};

	struct HomingCameraSettings
	{
		float cameraDistance{ 900.0f };
		float cameraHeight{ 420.0f };
		float targetHeight{ 110.0f };
		float followSpeed{ 5.0f };
	};

	class HomingCamera : public Camera
	{
	public:
		void Reset() override
		{
			Camera::Reset();
			position_ = { 400.0f, 400.0f, 400.0f };
			target_ = {};
		}

		void Update(const Vec3& playerPosition, const Vec3& bossPosition, float deltaTime)
		{
			Vec3 awayFromBoss = playerPosition - bossPosition;
			awayFromBoss.y = 0.0f;
			if (awayFromBoss.LengthSq() < 0.001f)
				awayFromBoss = { 0.0f, 0.0f, 1.0f };
			else
				awayFromBoss = awayFromBoss.Normalized();

			const Vec3 desiredPosition = playerPosition +
				awayFromBoss * settings_.cameraDistance +
				Vec3(0.0f, settings_.cameraHeight, 0.0f);
			const Vec3 desiredTarget = (playerPosition + bossPosition) * 0.5f +
				Vec3(0.0f, settings_.targetHeight, 0.0f);
			const float followAmount = std::clamp(settings_.followSpeed * deltaTime, 0.0f, 1.0f);
			position_ += (desiredPosition - position_) * followAmount;
			target_ += (desiredTarget - target_) * followAmount;
		}

		void SetTarget(const Vec3& target) { target_ = target; }
		const Vec3& GetTarget() const { return target_; }
		HomingCameraSettings& GetSettings() { return settings_; }
		const HomingCameraSettings& GetSettings() const { return settings_; }

		void ApplyCamera() const override { ApplyView(target_); }

	private:
		Vec3 target_{};
		HomingCameraSettings settings_{};
	};

}