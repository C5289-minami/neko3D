#pragma once
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
		Init();
      motion.Start(0.0f, 1.0f, motionDuration, showEasing);
		SetToMove(dir); // モーションの方向を設定
		state = State::Do;// モーション実行中に設定
	}
	virtual ~UIBase() = default;

	virtual void Init();
	virtual void Update(float deltaTime);
	virtual void Draw();

	// 方向を入れると出てくるMotionを設定する関数
	void SetToMove(DxPlus::Vec2 dir);
	void UpdateStartMotion();
    // 表示・非表示モーションのイージングを設定する
	void SetShowEasing(Motion::Easing easing) { showEasing = easing; if (state == State::Do) motion.SetEasing(easing); }
	void SetHideEasing(Motion::Easing easing) { hideEasing = easing; if (state == State::Hide) motion.SetEasing(easing); }
  void SetMotionDuration(float duration) { motion.SetDuration(duration); UpdateStartMotion(); }
	float GetMotionDuration() const { return motion.GetDuration(); }
    void SetMoveDistance(float distance) { MoveDistance = std::max(0.0f, distance); UpdateStartMotion(); }
	float GetMoveDistance() const { return MoveDistance; }
	// alpha
  void SetAlpha(int targetAlpha) { finAlpha = std::clamp(targetAlpha, 0, 255); }
	int GetAlpha() const { return alpha; }

	virtual void SetScale(float targetScale) { scale = targetScale; }
	float GetScale() const { return scale; }
	bool IsActive() const { return state != State::False; }
	void SetFalse() { state = State::False; }

	// 表示条件を設定する関数
    void SetDisplayCondition(std::function<bool()> condition)
	{
		displayCondition = condition ? std::move(condition) : [this]() { return state != State::False; };
	}

	// Zオーダー(描画順)を設定する関数
	void SetRecord(int z) { zOrder = z; } // for backward compatibility or future use, wait let me just use SetZOrder
	void SetZOrder(int z) { zOrder = z; }
	int GetZOrder() const { return zOrder; }

protected:
	DxPlus::Vec2 position{};
	DxPlus::Vec2 basePosition{};
	float scale{ 1.0f };
    std::wstring key;
	int alpha{ 255 };
	int finAlpha{ 255 };
	enum class State
	{
		Normal,// ふつうの状態(静止)
		Do,// モーション実行中(出現)
		Hide,// 消失モーション中
		False,// 表示していない
	};
	State state{ State::Normal };
	//表示する条件
	std::function<bool()> displayCondition = [this]() { return state != State::False; };

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