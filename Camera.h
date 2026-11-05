
#pragma once

#include "Vector3.h"
#include "DxLib.h"
#include "DxConv.h"
#include "DxPlus/DxPlus.h"
#include "CameraPose.h"
#include "Consts.h"

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

		static constexpr float DefaultFieldOfView = 60.0f;

		static float GetFieldOfView() { return fieldOfViewDegrees_; }

		static void SetFieldOfView(float degrees)
		{
			if (std::isfinite(degrees))
				fieldOfViewDegrees_ =
				std::clamp(degrees, 10.0f, 150.0f);
		}

		virtual void Initialize(const Vec3& eye, const Vec3& target)
		{
			position_ = eye;
			roll_ = 0.0f;

			const Vec3 direction =
				(target - eye).Normalized();

			pitch_ = std::asin(
				std::clamp(direction.y, -1.0f, 1.0f));

			yaw_ = std::atan2(
				direction.x, direction.z);

		}
		virtual void Initialize(const CameraPose& pose)
		{
			Vec3 direction{
				std::sin(pose.yaw) * std::cos(pose.pitch),
				std::sin(pose.pitch),
				std::cos(pose.yaw) * std::cos(pose.pitch)
			};

			Initialize(pose.position, pose.position + direction);
			roll_ = pose.roll;
		}

		virtual void Reset()
		{
			position_ = {};
			yaw_ = 0.0f;
			pitch_ = 0.0f;
			roll_ = 0.0f;
		}

		virtual void Update(float) {}

		// 現在のカメラ姿勢を取得
		virtual CameraPose GetPose() const;


		// 指定された姿勢をDxLibの描画用カメラに適用する
		// カメラ内部の状態は変更しない
		void ApplyPose(const CameraPose& pose) const;

		// カメラの姿勢を描画用カメラに適用する
		virtual void ApplyCamera() const;

		void SetYaw(float yaw) { yaw_ = yaw; }
		void SetPitch(float pitch) { pitch_ = pitch; }
		const Vec3& GetPosition() const { return position_; }
		void SetPosition(const Vec3& position) { position_ = position; }
		float GetYaw() const { return yaw_; }
		float GetPitch() const { return pitch_; }

		// カメラの角度を度数で取得する
		Vec3 GetAnglesDegrees() const
		{
			return {
				pitch_ / DxPlus::Deg2Rad,
				yaw_ / DxPlus::Deg2Rad,
				roll_ / DxPlus::Deg2Rad
			};
		}
		// カメラの角度を度数で設定する
		void SetAnglesDegrees(const Vec3& angles)
		{
			if (!std::isfinite(angles.x) ||
				!std::isfinite(angles.y) ||
				!std::isfinite(angles.z))
			{
				return;
			}

			pitch_ = std::clamp(
				angles.x, -89.0f, 89.0f) * DxPlus::Deg2Rad;

			yaw_ = std::remainder(
				angles.y, 360.0f) * DxPlus::Deg2Rad;

			roll_ = std::remainder(
				angles.z, 360.0f) * DxPlus::Deg2Rad;
		}

		// カメラの描画範囲
		static float GetNearClipDistance();
		static float GetFarClipDistance();

		static bool SetNearFar(
			float nearDistance,
			float farDistance);
	protected:
		// 注視点を指定してカメラを適用する
		void ApplyView(const Vec3& target) const;

		inline static float nearClipDistance_{ Const::CAMERA_NEAR };
		inline static float farClipDistance_{ Const::CAMERA_FAR };
		inline static float fieldOfViewDegrees_{ DefaultFieldOfView };

		Vec3 position_{};

		float yaw_{};
		float pitch_{};
		float roll_{};
	};


	// =========================================================
	// FreeCamera
	// デバッグ用の自由移動カメラ
	// =========================================================

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

		void Initialize(const Vec3& eye, const Vec3& target) override
		{
			Camera::Initialize(eye, target);
			homeEye_ = eye;
			homeTarget_ = target;

			active_ = false;
		}
		using Camera::Initialize;

		void Update(float deltaTime) override
		{
#ifndef NDEBUG
			const bool keyboardCaptured =
				ImGui::GetCurrentContext() &&
				ImGui::GetIO().WantCaptureKeyboard;

			const bool mouseCaptured =
				ImGui::GetCurrentContext() &&
				ImGui::GetIO().WantCaptureMouse;

			UpdateFreeFlight(
				deltaTime,
				keyboardCaptured,
				mouseCaptured);
#else
			UpdateFreeFlight(
				deltaTime,
				false,
				false);
#endif

			ApplyCamera();
		}

		void BeginSceneView()
		{
			if (active_)
				return;

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

		void FocusAt(
			const Vec3& point,
			float distance = 350.0f)
		{
			position_ =
				point - GetForward() * distance;
		}

		bool IsSceneViewActive() const { return active_; }

		const Vec3& GetEye() const { return position_; }

		Vec3 GetTarget() const { return GetForwardTarget(); }

		void UpdateSceneView(
			float deltaTime,
			const Vec3& focusPoint)
		{
#ifndef NDEBUG
			const bool keyboardCaptured =
				ImGui::GetCurrentContext() &&
				ImGui::GetIO().WantCaptureKeyboard;

			const bool mouseCaptured =
				ImGui::GetCurrentContext() &&
				ImGui::GetIO().WantCaptureMouse;

			const bool toggleDown =
				DxLib::CheckHitKey(KEY_INPUT_LALT) != 0 &&
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

			if (!active_)
				return;

			const bool resetDown =
				!keyboardCaptured &&
				DxLib::CheckHitKey(KEY_INPUT_R) != 0;

			if (resetDown && !wasResetDown_)
				ResetView();

			wasResetDown_ = resetDown;

			const bool focusDown =
				!keyboardCaptured &&
				DxLib::CheckHitKey(KEY_INPUT_F) != 0;

			if (focusDown && !wasFocusDown_)
				FocusAt(focusPoint);

			wasFocusDown_ = focusDown;

			UpdateFreeFlight(
				deltaTime,
				keyboardCaptured,
				mouseCaptured);
#else
			(void)deltaTime;
			(void)focusPoint;
#endif
		}

		void SetMoveSpeed(float speed) { moveSpeed_ = speed; }

		void SetRotateSpeed(float speed) { rotateSpeed_ = speed; }

		float GetMoveSpeed() const { return moveSpeed_; }

		float GetRotateSpeed() const { return rotateSpeed_; }

		void SetMoveShiftRatio(float ratio) { moveShiftRatio_ = ratio; }

	private:
		Vec3 GetForward() const
		{
			const float cosPitch = std::cos(pitch_);

			return {
				std::sin(yaw_) * cosPitch,
				std::sin(pitch_),
				std::cos(yaw_) * cosPitch
			};
		}

		Vec3 GetForwardTarget() const { return position_ + GetForward(); }

		void RestoreHomeView()
		{
			position_ = homeEye_;

			const Vec3 direction =
				(homeTarget_ - homeEye_).Normalized();

			pitch_ = std::asin(
				std::clamp(direction.y, -1.0f, 1.0f));

			yaw_ = std::atan2(
				direction.x, direction.z);
		}

		void UpdateFreeFlight(
			float deltaTime,
			bool keyboardCaptured,
			bool mouseCaptured)
		{
			const Vec3 forward = GetForward();

			const Vec3 right{
				std::cos(yaw_),
				0.0f,
				-std::sin(yaw_)
			};

			Vec3 movement{};

			if (!keyboardCaptured &&
				DxLib::CheckHitKey(KEY_INPUT_W))
			{
				movement += forward;
			}

			if (!keyboardCaptured &&
				DxLib::CheckHitKey(KEY_INPUT_S))
			{
				movement -= forward;
			}

			if (!keyboardCaptured &&
				DxLib::CheckHitKey(KEY_INPUT_A))
			{
				movement -= right;
			}

			if (!keyboardCaptured &&
				DxLib::CheckHitKey(KEY_INPUT_D))
			{
				movement += right;
			}

			if (!keyboardCaptured &&
				DxLib::CheckHitKey(KEY_INPUT_Q))
			{
				movement.y -= 1.0f;
			}

			if (!keyboardCaptured &&
				DxLib::CheckHitKey(KEY_INPUT_E))
			{
				movement.y += 1.0f;
			}

			if (movement.LengthSq() > 0.0f)
			{
				const float speed =
					moveSpeed_ *
					(DxLib::CheckHitKey(KEY_INPUT_LSHIFT)
						? moveShiftRatio_
						: 1.0f);

				position_ +=
					movement.Normalized() *
					(speed * deltaTime);
			}

			const DxPlus::Vec2Int mouseDelta =
				mouseCaptured
				? DxPlus::Vec2Int{}
			: DxPlus::Input::GetMouseDelta();

			yaw_ += mouseDelta.x * rotateSpeed_;

			pitch_ = std::clamp(
				pitch_ - mouseDelta.y * rotateSpeed_,
				-DxPlus::Deg2Rad * 89.0f,
				DxPlus::Deg2Rad * 89.0f);

			if (!mouseCaptured)
			{
				SetMousePoint(
					DxPlus::CLIENT_WIDTH / 2,
					DxPlus::CLIENT_HEIGHT / 2);
			}
		}

		float moveSpeed_{ 500.0f };
		float moveShiftRatio_{ 2.0f };

		float rotateSpeed_{
			0.2f * DxPlus::Deg2Rad
		};

		bool active_{};
		bool wasToggleDown_{};
		bool wasResetDown_{};
		bool wasFocusDown_{};

		Vec3 homeEye_{};
		Vec3 homeTarget_{};
	};


	// =========================================================
	// TargetCamera
	// プレイヤーとボスを基準にした追従カメラ
	// =========================================================

	struct TargetCameraSettings
	{
		float cameraDistance{ 600.0f };
		float cameraHeight{ 200.0f };
		float targetHeight{ 110.0f };
		float followSpeed{ 5.0f };
	};


	class TargetCamera : public Camera
	{
	public:
		void Reset() override
		{
			Camera::Reset();

			position_ = {
				400.0f,
				400.0f,
				400.0f
			};

			target_ = {};
			followPosition_ = position_;
			positionOffset_ = {};
			angleOffsetDegrees_ = {};
		}

		void Update(
			const Vec3& playerPosition,
			const Vec3& bossPosition,
			float deltaTime)
		{
			Vec3 awayFromBoss =
				playerPosition - bossPosition;

			// プレイヤーとボスがほぼ同じ位置にいる場合、正規化できないので、適当な方向を設定する
			if (awayFromBoss.LengthSq() < 1000.0f * Const::EPS)
			{
				awayFromBoss = {
					0.0f,
					0.0f,
					1.0f
				};
			}
			else
			{
				awayFromBoss = awayFromBoss.Normalized();
			}

			// プレイヤーとボスの中間点を注視点にする
			const Vec3 desiredPosition =
				playerPosition
				+ awayFromBoss * settings_.cameraDistance
				+ Vec3(
					0.0f,
					settings_.cameraHeight,
					0.0f);
			// プレイヤーとボスの中間点を注視点にする
			const Vec3 desiredTarget =
				(playerPosition + bossPosition) * 0.5f
				+ Vec3(
					0.0f,
					settings_.targetHeight,
					0.0f);
			// 追従カメラの位置を滑らかに更新
			const float followAmount =
				std::clamp(
					settings_.followSpeed * deltaTime,
					0.0f,
					1.0f);
			// 追従カメラの位置を滑らかに更新
			followPosition_ +=
				(desiredPosition - followPosition_) *
				followAmount;

			position_ =
				followPosition_ + positionOffset_;

			target_ +=
				(desiredTarget - target_) *
				followAmount;
		}

		// 追従カメラの姿勢を取得
		CameraPose GetPose() const override;

		const Vec3& GetAngleOffsetDegrees() const
		{
			return angleOffsetDegrees_;
		}

		Vec3 GetViewAnglesDegrees() const
		{
			const Vec3 direction =
				(target_ - position_).Normalized();

			return {
				std::clamp(
					std::asin(
						std::clamp(
							direction.y,
							-1.0f,
							1.0f))
					/ DxPlus::Deg2Rad
					+ angleOffsetDegrees_.x,
					-89.0f,
					89.0f),

				std::atan2(
					direction.x,
					direction.z)
					/ DxPlus::Deg2Rad
					+ angleOffsetDegrees_.y,

				angleOffsetDegrees_.z
			};
		}

		void SetViewPosition(const Vec3& position)
		{
			if (!std::isfinite(position.x) ||
				!std::isfinite(position.y) ||
				!std::isfinite(position.z))
			{
				return;
			}

			positionOffset_ =
				position - followPosition_;

			position_ =
				followPosition_ + positionOffset_;
		}

		void SetAngleOffsetDegrees(const Vec3& angles)
		{
			if (!std::isfinite(angles.x) ||
				!std::isfinite(angles.y) ||
				!std::isfinite(angles.z))
			{
				return;
			}

			angleOffsetDegrees_ = {
				std::clamp(angles.x, -89.0f, 89.0f),
				std::remainder(angles.y, 360.0f),
				std::remainder(angles.z, 360.0f)
			};

			roll_ =
				angleOffsetDegrees_.z *
				DxPlus::Deg2Rad;
		}

		void FocusAt(const Vec3& point)
		{
			const Vec3 direction =
				(point - position_).Normalized();

			if (direction.LengthSq() < Const::EPS)
				return;

			const Vec3 current =
				GetViewAnglesDegrees();

			SetAngleOffsetDegrees(
				angleOffsetDegrees_ + Vec3{
					std::asin(
						std::clamp(
							direction.y,
							-1.0f,
							1.0f))
						/ DxPlus::Deg2Rad
						- current.x,

					std::atan2(
						direction.x,
						direction.z)
						/ DxPlus::Deg2Rad
						- current.y,

					0.0f
				});
		}

		void SetTarget(const Vec3& target) { target_ = target; }

		const Vec3& GetTarget() const { return target_; }

		TargetCameraSettings& GetSettings() { return settings_; }

		const TargetCameraSettings& GetSettings() const { return settings_; }

	private:
		Vec3 target_{};

		TargetCameraSettings settings_{};

		Vec3 followPosition_{
			400.0f,
			400.0f,
			400.0f
		};

		Vec3 positionOffset_{};
		Vec3 angleOffsetDegrees_{};
	};
}
