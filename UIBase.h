#pragma once
#include <algorithm>
#include <string>
#include <utility>
#include "DxPlus/DxPlus.h"
#include <functional> 
#include "Motion.h"

class UIBase
{
public:
	UIBase(const std::wstring& key, DxPlus::Vec2 pos, DxPlus::Vec2 dir = DxPlus::Vec2(0, -1), float motionDuration = 1.0f) :
		key(key),
		position(pos),
		basePosition(pos)
	{
		//Init();
		motion.Start(0.0f, 1.0f, motionDuration, showEasing);
		SetToMove(dir); // モーションの方向を設定
		state = UIState::Do;// モーション実行中に設定
	}
	virtual ~UIBase() = default;

	virtual void Init();
	virtual void Update(float deltaTime);
	virtual void Draw();

	// 方向を入れると出てくるMotionを設定する関数
	void SetToMove(DxPlus::Vec2 dir);
	void UpdateStartMotion();
	// 表示・非表示モーションのイージングを設定する
	void SetShowEasing(Motion::Easing easing) { showEasing = easing; if (state == UIState::Do) motion.SetEasing(easing); }
	void SetHideEasing(Motion::Easing easing) { hideEasing = easing; if (state == UIState::Hide) motion.SetEasing(easing); }
	void SetMotionDuration(float duration) { motion.SetDuration(duration); UpdateStartMotion(); }
	float GetMotionDuration() const { return motion.GetDuration(); }
	void SetMoveDistance(float distance) { MoveDistance = std::max(0.0f, distance); UpdateStartMotion(); }
	float GetMoveDistance() const { return MoveDistance; }
	// alpha
	void SetAlpha(int targetAlpha)
	{
		finAlpha = std::clamp(targetAlpha, 0, 255);
		manualAlpha = false;
	}

	// 現在の透明度を直接設定する
	void SetAlphaDirect(int value)
	{
		alpha = std::clamp(value, 0, 255);
		manualAlpha = true;
	}
	int GetAlpha() const { return alpha; }
	void FadeOut(float duration = -1.0f);
	void FadeIn(float duration = -1.0f);
	void Shake(float intensity = 8.0f, float duration = 0.25f, float frequency = 25.0f);

	virtual void SetScale(float targetScale) { scale = targetScale; }
	float GetScale() const { return scale; }
	void SetPosition(DxPlus::Vec2 targetPosition)
	{
		basePosition = targetPosition;
		if (state == UIState::Do || state == UIState::Hide)
			UpdateStartMotion();
		else
			position = targetPosition;
	}
	DxPlus::Vec2 GetPosition() const { return basePosition; }
	const std::wstring& GetKey() const { return key; }
	bool IsActive() const { return state != UIState::False; }
	// 見えるかどうかを設定
	void SetVisible(bool visible) { isVisible = visible; fadeOnlyMotion = false; }
	bool IsVisible() const { return isVisible; }
	//　有効化
	void SetTrue()
	{
		isVisible = true;
		fadeOnlyMotion = false;

		motion.Start(0.0f, 1.0f, motion.GetDuration(), showEasing);
		state = UIState::Do;

		UpdateStartMotion();
	}
	// 無効化
	void SetFalse() { isVisible = false; state = UIState::False; }
	// 非表示モーションを開始する
	void Hide();

	// 表示条件を設定する関数
	void SetDisplayCondition(std::function<bool()> condition)
	{
		displayCondition = condition ? std::move(condition) : []() { return true; };
	}

	// Zオーダー(描画順)を設定する関数
	void SetRecord(int z) { zOrder = z; } // for backward compatibility or future use, wait let me just use SetZOrder
	void SetZOrder(int z) { zOrder = z; }
	int GetZOrder() const { return zOrder; }

	void SetCenter(DxPlus::Vec2 value) { center = value; }
	DxPlus::Vec2 GetCenter() const { return center; }

	void SetAngle(float value) { angle = value; }
	float GetAngle() const { return angle; }

	void SetAngleSpeed(float value) { angleSpeed = value; }
	float GetAngleSpeed() const { return angleSpeed; }


	enum class UIState
	{
		Normal,// ふつうの状態(静止)
		Do,// モーション実行中(出現)
		Hide,// 消失モーション中
		False,// 表示していない
	};
	UIState GetCurrentState()const { return state; }
protected:
	DxPlus::Vec2 position{};
	DxPlus::Vec2 basePosition{};
	DxPlus::Vec2 center{ 0.5f, 0.5f };
	float scale{ 1.0f };
	float angle{};
	float angleSpeed{};
	std::wstring key;
	int alpha{ 255 };
	int finAlpha{ 255 };
	bool manualAlpha{ false };
	
	UIState state{ UIState::Normal };
	//表示する条件
	std::function<bool()> displayCondition = []() { return true; };
	bool isVisible{ true };
	bool fadeOnlyMotion{};
	float shakeElapsed{};
	float shakeDuration{};
	float shakeIntensity{};
	float shakeFrequency{};
	DxPlus::Vec2 shakeOffset{};


	// ===============
	// 実行時間関連
	// ===============
	Motion motion;
	Motion::Easing showEasing{ Motion::Easing::EaseOut };
	Motion::Easing hideEasing{ Motion::Easing::EaseIn };

	// =======================
	// 表示開始モーション関連
	// =======================
	DxPlus::Vec2 Direction{}; // モーションの方向
	float MoveDistance{ 100.0f }; // モーションの移動距離

	int zOrder{ 0 }; // 描画順を設定（値が大きいほど手前に描画）
};