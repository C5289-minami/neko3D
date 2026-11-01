#pragma once

#include "AnimationHandle.h"
#include "Consts.h"

class ModelAnimation
{
public:
	ModelAnimation() = default;
	~ModelAnimation() = default;

	ModelAnimation(const ModelAnimation&) = delete;
	ModelAnimation& operator=(const ModelAnimation&) = delete;


	// 再生するアニメーションを設定する。
	void Play3D(
		int modelHandle,
		int animIndex,
		bool loop,
		float speedScale = Const::ANIM_FPS,
		float blendSeconds = 0.15f,
		bool playImmediately = true
	);

	void Play3D(
		int modelHandle,
		const AnimationHandle& animation,
		bool loop,
		float speedScale = Const::ANIM_FPS,
		float blendSeconds = 0.15f,
		bool playImmediately = true
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

	// 現在位置から指定時間まで再生して停止する。
	void PlayTo(float endTime, float speedScale = Const::ANIM_FPS);

	// 現在位置から最後まで再生する。
	void Start(float speedScale = Const::ANIM_FPS);

	// 現在位置で停止する。
	void Pause();

	// 指定時間へ移動して停止する。
	void SetTime(float time);

	void SetSpeedScale(float speedScale)
	{
		speedScale_ = std::isfinite(speedScale)
			? (std::max)(0.0f, speedScale)
			: Const::ANIM_FPS;
	}
private:
	class AttachedAnimation
	{
	public:
		AttachedAnimation() = default;
		~AttachedAnimation();

		AttachedAnimation(const AttachedAnimation&) = delete;
		AttachedAnimation& operator=(const AttachedAnimation&) = delete;

		bool Attach(int targetModelHandle, const AnimationHandle& animation);
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

	// 現在のアニメーション
	AttachedAnimation currentAnimation_;
	AnimationHandle currentAnimationHandle_;
	float currentElapsed_{ 0.0f };

	// 切り替え先のアニメーション
	AttachedAnimation nextAnimation_;
	AnimationHandle nextAnimationHandle_;

	// ブレンドの進行状況
	float blendElapsed_{ 0.0f };
	float blendDuration_{ 0.0f };

	// ブレンド完了後に自動再生するか
	bool nextPlayImmediately_{ true };

	// ブレンド中にStart()が呼ばれたか
	bool startRequested_{ false };
	// ブレンド中にPlayTo()が呼ばれた場合の予約
	bool playToRequested_{ false };
	float requestedEndTime_{ -1.0f };
	float requestedSpeedScale_{ Const::ANIM_FPS };
	// 現在のアニメーションが再生中か
	bool isPlaying_{ true };
	// playEndTime_ に到達したら自動停止。-1.0f ならアニメーション全体の最後まで再生する。
	float playEndTime_{ -1.0f };

	bool loop_{ true };
	float speedScale_{ Const::ANIM_FPS };

	bool finished_{ false };
};	