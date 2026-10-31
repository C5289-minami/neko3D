#pragma once

#include "AnimationHandle.h"

class ModelAnimation
{
public:
	ModelAnimation() = default;
	~ModelAnimation() = default;

	ModelAnimation(const ModelAnimation&) = delete;
	ModelAnimation& operator=(const ModelAnimation&) = delete;


	// アニメーション番号で再生する。
	void Play3D(
		int modelHandle,
		int animIndex,
		bool loop,
		float speedScale = 1.0f,
		float blendSeconds = 0.15f
	);

	// AnimationHandleで再生する。
	void Play3D(
		int modelHandle,
		const AnimationHandle& animation,
		bool loop,
		float speedScale = 1.0f,
		float blendSeconds = 0.15f
	);

	// 再生時間とブレンドを更新する。
	void Update(float deltaTime);

	// アニメーションを解除する。
	void Reset();

	AnimationHandle GetCurrentAnimationHandle() const
	{
		return currentAnimationHandle_;
	}

	bool IsBlending() const { return static_cast<bool>(nextAnimation_); }
	bool IsFinished() const;
private:
	class AttachedAnimation
	{
	public:
		AttachedAnimation() = default;
		~AttachedAnimation();

		AttachedAnimation(const AttachedAnimation&) = delete;
		AttachedAnimation& operator=(const AttachedAnimation&) = delete;

		bool Attach(
			int targetModelHandle,
			const AnimationHandle& animation
		);

		void Reset();

		explicit operator bool() const { return attachIndex_ >= 0; }

		int GetModelHandle() const { return targetModelHandle_; }
		int GetAttachIndex() const { return attachIndex_; }

	private:
		int targetModelHandle_{ -1 };
		int sourceModelHandle_{ -1 };
		int animIndex_{ -1 };
		int attachIndex_{ -1 };
	};

	static void AdvanceAnimation(
		const AttachedAnimation& animation,
		float& elapsed,
		bool loop,
		float speedScale,
		float deltaTime
	);

	static void SetAnimationTime(
		const AttachedAnimation& animation,
		float elapsed
	);

	// 再生対象のモデル
	int modelHandle_{ -1 };

	// 現在再生しているアニメーション
	AttachedAnimation currentAnimation_;
	AnimationHandle currentAnimationHandle_;
	float currentElapsed_{ 0.0f };

	// 切り替え先のアニメーション
	AttachedAnimation nextAnimation_;
	AnimationHandle nextAnimationHandle_;
	float nextElapsed_{ 0.0f };

	// ブレンドの進行状況
	float blendElapsed_{ 0.0f };
	float blendDuration_{ 0.0f };

	bool loop_{ true };
	float speedScale_{ 1.0f };
	bool finished_{ false };
};