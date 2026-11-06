
#pragma once

#include "Camera.h"
#include "CameraTransition.h"
#include "Motion.h"
#include "Collision.h"
#include "CameraShake.h"

#include <vector>
#include <string>
#include <unordered_map>

namespace Camera
{
	// カメラの切り替え・演出・遷移を管理する
	class CameraDirector
	{
	public:
		explicit CameraDirector(Camera& camera);

		// 演出アングルを登録
		bool RegisterShot(
			const std::string& name,
			const CameraPose& pose);

		// 登録した演出アングルを再生
		bool PlayShot(
			const std::string& name,
			float duration,
			Motion::Easing easing = Motion::Easing::EaseInOut);

		// 任意の姿勢へ移動（割り込み可能）
		void MoveTo(
			const CameraPose& targetPose,
			float duration,
			Motion::Easing easing = Motion::Easing::EaseInOut);

		// 使用するカメラを切り替える
		void SwitchCamera(
			Camera& targetCamera,
			float duration = 0.0f,
			Motion::Easing easing = Motion::Easing::EaseInOut);

		// 現在のカメラ本来の姿勢へ戻る
		void Follow(
			float duration = 0.0f,
			Motion::Easing easing = Motion::Easing::EaseInOut);

		// 前のカメラへ戻る、または演出を終了する
		void ReturnToPreviousCamera(
			float duration = 0.0f,
			Motion::Easing easing = Motion::Easing::EaseInOut);

		// カメラの現在姿勢を即時更新する
		void SetPoseImmediate(const CameraPose& pose);

		// 毎フレーム更新。遷移完了時のみ true
		bool Update(float deltaTime);

		// 初期状態へ戻す
		void Reset();

		// 現在の描画用姿勢
		const CameraPose& GetCurrentPose() const;

		// 描画パスで同じカメラ姿勢を再適用する
		void ApplyRenderPose() const;

		// 現在使用中のカメラ
		Camera* GetActiveCamera() const;

		// 遷移中か
		bool IsTransitioning() const;

		// カメラ本来の姿勢を使用中か
		bool IsFollowing() const;

		// カメラ揺れ
		void StartHorizontalShake(
			float duration = 0.25f,
			float strength = 20.0f,
			float frequency = 20.0f)
		{
			cameraShake_.StartHorizontal(duration, strength, frequency);
		}

		void StartVerticalShake(
			float duration = 0.25f,
			float strength = 20.0f,
			float frequency = 20.0f)
		{
			cameraShake_.StartVertical(duration, strength, frequency);
		}

		void StartMixedShake(
			float duration = 0.25f,
			float strength = 20.0f,
			float frequency = 20.0f)
		{
			cameraShake_.StartMixed(duration, strength, frequency);
		}

		void StopShake() { cameraShake_.Stop(); }

		// 現在のカメラ遷移Progress（0.0～1.0）
		float GetTransitionProgress() const;

	private:
		enum class Mode
		{
			FollowingCamera,
			MovingToPose,
			HoldingPose,
			BlendingToCamera
		};

		// 現在のカメラへ滑らかに戻る
		void BeginBlendToActiveCamera(
			float duration,
			Motion::Easing easing);

		// カメラ本来の姿勢を取得
		CameraPose GetBasePose() const;

		// 姿勢を補間
		static CameraPose InterpolatePose(
			const CameraPose& from,
			const CameraPose& to,
			float t);

		// 角度を最短経路で補間（ラジアン）
		static float LerpAngle(
			float from,
			float to,
			float t);

		// 姿勢の数値が有効か
		static bool IsFinitePose(const CameraPose& pose);

		Camera* activeCamera_{};
		Camera* previousCamera_{};

		std::unordered_map<std::string, CameraPose> shots_{};

		CameraTransition transition_{};
		Motion followMotion_{};

		CameraPose currentPose_{};
		CameraPose renderPose_{};
		CameraPose followStartPose_{};

		Mode mode_{ Mode::FollowingCamera };

		// 描画時にカメラ姿勢へ揺れを加える
		void ApplyCurrentPose(float deltaTime);

		CameraShake cameraShake_{};

	};
}
