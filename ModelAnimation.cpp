#include "ModelAnimation.h"
#include "DxLib.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <tuple>

namespace
{
	using AnimationKey = std::tuple<int, int, int>;

	struct SharedAttachInfo
	{
		int attachIndex;
		int referenceCount;
	};

	std::map<AnimationKey, SharedAttachInfo> sharedAttaches;
}

// ============================================================
// AttachedAnimation
// ============================================================

ModelAnimation::AttachedAnimation::~AttachedAnimation()
{
	Reset();
}

bool ModelAnimation::AttachedAnimation::Attach(
	int targetModelHandle,
	const AnimationHandle& animation)
{
	if (targetModelHandle < 0 || !animation)
		return false;

	const int sourceModelHandle = animation.GetSourceModelHandle();
	const int animIndex = animation.GetAnimIndex();

	if (sourceModelHandle < 0 ||
		animIndex < 0 ||
		animIndex >= MV1GetAnimNum(sourceModelHandle))
	{
		return false;
	}

	if (targetModelHandle_ == targetModelHandle &&
		sourceModelHandle_ == sourceModelHandle &&
		animIndex_ == animIndex &&
		attachIndex_ >= 0)
	{
		return true;
	}

	Reset();

	const AnimationKey key{
		targetModelHandle,
		sourceModelHandle,
		animIndex
	};

	auto it = sharedAttaches.find(key);

	if (it != sharedAttaches.end())
	{
		++it->second.referenceCount;

		targetModelHandle_ = targetModelHandle;
		sourceModelHandle_ = sourceModelHandle;
		animIndex_ = animIndex;
		attachIndex_ = it->second.attachIndex;

		return true;
	}

	const int attachIndex = MV1AttachAnim(
		targetModelHandle,
		animIndex,
		sourceModelHandle,
		FALSE
	);

	if (attachIndex < 0)
		return false;

	sharedAttaches.emplace(
		key,
		SharedAttachInfo{ attachIndex, 1 }
	);

	targetModelHandle_ = targetModelHandle;
	sourceModelHandle_ = sourceModelHandle;
	animIndex_ = animIndex;
	attachIndex_ = attachIndex;

	return true;
}

void ModelAnimation::AttachedAnimation::Reset()
{
	if (targetModelHandle_ >= 0 &&
		sourceModelHandle_ >= 0 &&
		animIndex_ >= 0 &&
		attachIndex_ >= 0)
	{
		const AnimationKey key{
			targetModelHandle_,
			sourceModelHandle_,
			animIndex_
		};

		auto it = sharedAttaches.find(key);

		if (it != sharedAttaches.end() &&
			it->second.attachIndex == attachIndex_)
		{
			--it->second.referenceCount;

			if (it->second.referenceCount == 0)
			{
				MV1DetachAnim(targetModelHandle_, attachIndex_);
				sharedAttaches.erase(it);
			}
		}
	}

	targetModelHandle_ = -1;
	sourceModelHandle_ = -1;
	animIndex_ = -1;
	attachIndex_ = -1;
}

// ============================================================
// Play3D
// ============================================================

void ModelAnimation::Play3D(
	int modelHandle,
	int animIndex,
	bool loop,
	float speedScale,
	float blendSeconds,
	bool playImmediately,
	Motion::Easing blendEasing)
{
	Play3D(
		modelHandle,
		AnimationHandle::FromIndex(modelHandle, animIndex),
		loop,
		speedScale,
		blendSeconds,
		playImmediately,
		blendEasing
	);
}

void ModelAnimation::Play3D(
	int modelHandle,
	const AnimationHandle& animation,
	bool loop,
	float speedScale,
	float blendSeconds,
	bool playImmediately,
	Motion::Easing blendEasing)
{
	if (modelHandle < 0 || !animation)
		return;

	speedScale = std::isfinite(speedScale)
		? (std::max)(0.0f, speedScale)
		: 1.0f;

	blendSeconds = std::isfinite(blendSeconds)
		? (std::max)(0.0f, blendSeconds)
		: 0.0f;

	// 初回再生、またはモデルが変わった場合。
	if (modelHandle_ != modelHandle || !currentAnimation_)
	{
		Reset();

		if (!currentAnimation_.Attach(modelHandle, animation))
			return;

		modelHandle_ = modelHandle;
		currentAnimationHandle_ = animation;
		currentElapsed_ = 0.0f;

		loop_ = loop;
		speedScale_ = speedScale;
		isPlaying_ = playImmediately;
		finished_ = false;

		MV1SetAttachAnimBlendRate(
			modelHandle_,
			currentAnimation_.GetAttachIndex(),
			1.0f
		);

		SetAnimationTime(currentAnimation_, 0.0f);
		return;
	}

	loop_ = loop;

	// 同じ切り替え先が指定され続けた場合。
	if (nextAnimation_ && nextAnimationHandle_ == animation)
	{
		nextPlayImmediately_ = playImmediately;
		return;
	}

	// 現在のアニメーションに戻す場合。
	if (currentAnimationHandle_ == animation)
	{
		if (nextAnimation_)
		{
			nextAnimation_.Reset();
			nextAnimationHandle_ = {};

			blendMotion_.Reset();

			nextPlayImmediately_ = true;
			startRequested_ = false;
			playToRequested_ = false;

			MV1SetAttachAnimBlendRate(
				modelHandle_,
				currentAnimation_.GetAttachIndex(),
				1.0f
			);
		}

		// 再生終了済みなら先頭から再生し直す。
		if (finished_)
		{
			currentElapsed_ = 0.0f;
			speedScale_ = speedScale;
			loop_ = loop;

			playEndTime_ = -1.0f;
			isPlaying_ = playImmediately;
			finished_ = false;

			SetAnimationTime(currentAnimation_, 0.0f);
		}

		return;
	}

	// ブレンド時間が0なら即座に切り替える。
	if (blendSeconds <= 0.0f)
	{
		nextAnimation_.Reset();
		currentAnimation_.Reset();

		if (!currentAnimation_.Attach(modelHandle, animation))
		{
			Reset();
			return;
		}

		currentAnimationHandle_ = animation;
		currentElapsed_ = 0.0f;

		blendMotion_.Reset();

		nextAnimationHandle_ = {};
		nextPlayImmediately_ = true;
		startRequested_ = false;
		playToRequested_ = false;

		speedScale_ = speedScale;
		isPlaying_ = playImmediately;
		finished_ = false;
		playEndTime_ = -1.0f;

		MV1SetAttachAnimBlendRate(
			modelHandle_,
			currentAnimation_.GetAttachIndex(),
			1.0f
		);

		SetAnimationTime(currentAnimation_, 0.0f);
		return;
	}

	// 別の切り替え先へ変更された場合。
	if (nextAnimation_)
	{
		nextAnimation_.Reset();
		nextAnimationHandle_ = {};

		blendMotion_.Reset();

		nextPlayImmediately_ = true;
		startRequested_ = false;
		playToRequested_ = false;

		MV1SetAttachAnimBlendRate(
			modelHandle_,
			currentAnimation_.GetAttachIndex(),
			1.0f
		);
	}

	if (!nextAnimation_.Attach(modelHandle, animation))
		return;

	nextAnimationHandle_ = animation;
	nextPlayImmediately_ = playImmediately;

	startRequested_ = false;
	playToRequested_ = false;

	blendMotion_.Start(
		0.0f,
		1.0f,
		blendSeconds,
		blendEasing
	);

	SetAnimationTime(nextAnimation_, 0.0f);

	MV1SetAttachAnimBlendRate(
		modelHandle_,
		currentAnimation_.GetAttachIndex(),
		1.0f
	);

	MV1SetAttachAnimBlendRate(
		modelHandle_,
		nextAnimation_.GetAttachIndex(),
		0.0f
	);
}

// ============================================================
// Play control
// ============================================================

void ModelAnimation::Start(float speedScale)
{
	speedScale_ = std::isfinite(speedScale)
		? (std::max)(0.0f, speedScale)
		: 1.0f;

	// ブレンド中なら、完了後に再生する。
	if (nextAnimation_)
	{
		startRequested_ = true;
		playToRequested_ = false;
		return;
	}

	if (!currentAnimation_)
		return;

	// 最後まで再生済みなら先頭から再生する。
	const float totalTime = MV1GetAttachAnimTotalTime(
		currentAnimation_.GetModelHandle(),
		currentAnimation_.GetAttachIndex()
	);

	if (totalTime > 0.0f && currentElapsed_ >= totalTime)
		currentElapsed_ = 0.0f;

	playEndTime_ = -1.0f;
	isPlaying_ = true;
	finished_ = false;

	SetAnimationTime(currentAnimation_, currentElapsed_);
}

void ModelAnimation::PlayTo(float endTime, float speedScale, float blendSeconds)
{
  if (!std::isfinite(endTime) ||
		!std::isfinite(speedScale) ||
		(!std::isfinite(blendSeconds) && blendSeconds != -1.0f))
		return;

	endTime = (std::max)(0.0f, endTime);
	speedScale = (std::max)(0.0f, speedScale);
	if (blendSeconds >= 0.0f)
		blendSeconds = (std::max)(0.0f, blendSeconds);

	// ブレンド中なら、完了後に指定位置まで再生する。
	if (nextAnimation_)
	{
		playToRequested_ = true;
		startRequested_ = false;

		requestedEndTime_ = endTime;
		requestedSpeedScale_ = speedScale;
		if (blendSeconds >= 0.0f)
		{
			blendMotion_.SetDuration(blendSeconds);
		}
		return;
	}

	if (!currentAnimation_)
		return;

	// 同じ終了位置ですでに停止しているなら何もしない。
	if (playEndTime_ == endTime && finished_)
		return;

	const float totalTime = MV1GetAttachAnimTotalTime(
		currentAnimation_.GetModelHandle(),
		currentAnimation_.GetAttachIndex()
	);

	playEndTime_ = (std::min)(endTime, totalTime);
	speedScale_ = speedScale;

	isPlaying_ = currentElapsed_ < playEndTime_;
	finished_ = !isPlaying_;

	if (!isPlaying_)
		currentElapsed_ = playEndTime_;

	SetAnimationTime(currentAnimation_, currentElapsed_);
}

void ModelAnimation::Pause()
{
	if (nextAnimation_)
	{
		startRequested_ = false;
		playToRequested_ = false;
		return;
	}

	isPlaying_ = false;
}

void ModelAnimation::SetTime(float time)
{
	if (!std::isfinite(time))
		return;

	if (nextAnimation_)
	{
		startRequested_ = false;
		playToRequested_ = false;
		return;
	}

	if (!currentAnimation_)
		return;

	const float totalTime = MV1GetAttachAnimTotalTime(
		currentAnimation_.GetModelHandle(),
		currentAnimation_.GetAttachIndex()
	);

	currentElapsed_ = (std::clamp)(time, 0.0f, totalTime);

	playEndTime_ = -1.0f;
	isPlaying_ = false;
	finished_ = false;

	SetAnimationTime(currentAnimation_, currentElapsed_);
}

// ============================================================
// Update
// ============================================================

void ModelAnimation::Update(float deltaTime)
{
	if (!std::isfinite(deltaTime))
		return;

	deltaTime = (std::max)(0.0f, deltaTime);

	if (!currentAnimation_)
		return;

	if (isPlaying_)
	{
		// PlayTo中はループさせず、指定時間まで進める。
		const bool useLoop = playEndTime_ < 0.0f && loop_;

		AdvanceAnimation(
			currentAnimation_,
			currentElapsed_,
			useLoop,
			speedScale_,
			deltaTime
		);

		// 指定時間に到達したら停止。
		if (playEndTime_ >= 0.0f && currentElapsed_ >= playEndTime_)
		{
			currentElapsed_ = playEndTime_;
			isPlaying_ = false;
			finished_ = true;
		}

		// アニメーション全体の最後に到達した場合。
		else if (!loop_)
		{
			const float totalTime = MV1GetAttachAnimTotalTime(
				currentAnimation_.GetModelHandle(),
				currentAnimation_.GetAttachIndex()
			);

			if (totalTime > 0.0f && currentElapsed_ >= totalTime)
			{
				currentElapsed_ = totalTime;
				isPlaying_ = false;
				finished_ = true;
			}
		}

		SetAnimationTime(currentAnimation_, currentElapsed_);
	}

	if (!nextAnimation_)
		return;

	// ブレンド中の次のアニメーションは先頭フレームで固定。
	SetAnimationTime(nextAnimation_, 0.0f);

	blendMotion_.Update(deltaTime);

	const float blendRate = blendMotion_.GetValue();


	MV1SetAttachAnimBlendRate(
		modelHandle_,
		currentAnimation_.GetAttachIndex(),
		1.0f - blendRate
	);

	MV1SetAttachAnimBlendRate(
		modelHandle_,
		nextAnimation_.GetAttachIndex(),
		blendRate
	);

	if (blendMotion_.IsFinished())
	{
		const AnimationHandle finishedAnimation = nextAnimationHandle_;

		nextAnimation_.Reset();
		currentAnimation_.Reset();

		if (!currentAnimation_.Attach(modelHandle_, finishedAnimation))
		{
			Reset();
			return;
		}

		currentAnimationHandle_ = finishedAnimation;
		currentElapsed_ = 0.0f;

		MV1SetAttachAnimBlendRate(
			modelHandle_,
			currentAnimation_.GetAttachIndex(),
			1.0f
		);

		SetAnimationTime(currentAnimation_, 0.0f);

		// ブレンド後の再生方法を決める。
		if (playToRequested_)
		{
			playEndTime_ = requestedEndTime_;
			speedScale_ = requestedSpeedScale_;

			const float totalTime = MV1GetAttachAnimTotalTime(
				currentAnimation_.GetModelHandle(),
				currentAnimation_.GetAttachIndex()
			);

			playEndTime_ = (std::min)(playEndTime_, totalTime);

			isPlaying_ = currentElapsed_ < playEndTime_;
			finished_ = !isPlaying_;
		}
		else
		{
			playEndTime_ = -1.0f;
			isPlaying_ = nextPlayImmediately_ || startRequested_;
			finished_ = false;
		}

		nextAnimationHandle_ = {};

		blendMotion_.Reset();

		nextPlayImmediately_ = true;
		startRequested_ = false;
		playToRequested_ = false;
	}
}

// ============================================================
// Animation time
// ============================================================

void ModelAnimation::AdvanceAnimation(
	const AttachedAnimation& animation,
	float& elapsed,
	bool loop,
	float speedScale,
	float deltaTime)
{
	if (!animation)
		return;

	const float totalTime = MV1GetAttachAnimTotalTime(
		animation.GetModelHandle(),
		animation.GetAttachIndex()
	);

	if (totalTime <= 0.0f)
		return;

	elapsed += deltaTime * speedScale;

	if (loop)
	{
		elapsed = std::fmod(elapsed, totalTime);

		if (elapsed < 0.0f)
			elapsed += totalTime;
	}
	else
	{
		elapsed = (std::min)(elapsed, totalTime);
	}
}

void ModelAnimation::SetAnimationTime(
	const AttachedAnimation& animation,
	float elapsed)
{
	if (!animation)
		return;

	MV1SetAttachAnimTime(
		animation.GetModelHandle(),
		animation.GetAttachIndex(),
		elapsed
	);
}

// ============================================================
// Reset
// ============================================================

void ModelAnimation::Reset()
{
	nextAnimation_.Reset();
	currentAnimation_.Reset();

	modelHandle_ = -1;

	currentAnimationHandle_ = {};
	currentElapsed_ = 0.0f;

	nextAnimationHandle_ = {};

	blendMotion_.Reset();

	nextPlayImmediately_ = true;
	startRequested_ = false;

	playToRequested_ = false;
	requestedEndTime_ = -1.0f;

	requestedSpeedScale_ = Const::ANIM_FPS;

	isPlaying_ = true;
	playEndTime_ = -1.0f;

	loop_ = true;
	speedScale_ = Const::ANIM_FPS;
	finished_ = false;
}

// ============================================================
// IsFinished
// ============================================================

bool ModelAnimation::IsFinished() const
{
	return finished_ && !nextAnimation_;
}