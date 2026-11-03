
#include "TutorialChecklist.h"
#include "UIManager.h"
#include "ResourceKeys.h"

#include <utility>

namespace
{
	// =========================
	// スタンプ演出時間
	// =========================
	constexpr float StampPressDuration = 1.5f;
	constexpr float StampFadeDuration = 0.3f;

	// =========================
	// 押印演出（Press）
	// =========================
	constexpr float PressStartAlpha = 0.0f;
	constexpr float PressEndAlpha = 255.0f;

	constexpr float PressStartScale = 5.0f;
	constexpr float PressEndScale = 1.0f;

	constexpr float PressStartAngleSpeed = 10.0f;
	constexpr float PressStartRandamAngle = 360.0f;
	constexpr float PressEndAngleSpeed = 0.0f;

	constexpr Motion::Easing PressEasing = Motion::Easing::EaseInOut;

	// =========================
	// 消失演出（FadeOut）
	// =========================
	constexpr float FadeStartAlpha = PressEndAlpha;
	constexpr float FadeEndAlpha = 0.0f;

	constexpr float FadeStartScale = PressEndScale;
	constexpr float FadeEndScale = 2.0f;

	constexpr float FadeStartAngleSpeed = PressEndAngleSpeed;
	constexpr float FadeEndAngleSpeed = 10.0f;

	constexpr Motion::Easing FadeEasing = Motion::Easing::EaseOut;

	// =========================
	// 演出用スタンプ初期値
	// =========================
	constexpr float EffectStartAngle = 0.0f;

	// =========================
	// 位置
	// =========================
	constexpr int StampRandomOffsetX = 100;

	// =========================
	// 補間
	// =========================
	float Lerp(float from, float to, float t)
	{
		return from + (to - from) * t;
	}
}

void TutorialChecklist::Init()
{
	items.clear();

	for (const auto& id : stampIds)
		UIM().RemoveUI(id);

	for (const auto& id : stampEffectIds)
		UIM().RemoveUI(id);

	stampIds.clear();
	stampEffectIds.clear();

	isVisible = false;

	panelPosition = DxPlus::Vec2(
		DxPlus::CLIENT_WIDTH * 0.25f,
		DxPlus::CLIENT_HEIGHT * 0.5f);

	auto* list = UIM().GetUI(L"TutorialList");

	if (!list)
	{
		list = UIM().CreateImageUI(
			L"TutorialList",
			ResourceKeys::TutorialBack,
			panelPosition,
			{ 0,-1 }
		);
	}

	if (list)
		list->SetFalse();
}

void TutorialChecklist::Update(float deltaTime)
{
	if (!isVisible) return;

	for (std::size_t i = 0; i < items.size(); ++i)
	{
		auto& item = items[i];

		if (item.stampPhase == StampAnimationPhase::None ||
			item.stampPhase == StampAnimationPhase::Finished)
			continue;

		const std::wstring effectId =
			L"TutorialStampEffect" + std::to_wstring(i);

		const std::wstring stampId =
			L"TutorialStamp" + std::to_wstring(i);

		auto* effect = UIM().GetUI(effectId);
		if (!effect) continue;

		auto* completedStamp = UIM().GetUI(stampId);

		item.stampMotion.Update(deltaTime);

		const float t = item.stampMotion.GetValue();

		// =================================
		// ① 押印演出
		// =================================
		if (item.stampPhase == StampAnimationPhase::Press)
		{
			const float alpha =
				Lerp(PressStartAlpha, PressEndAlpha, t);

			item.checkScale =
				Lerp(PressStartScale, PressEndScale, t);

			const float angleSpeed =
				Lerp(PressStartAngleSpeed, PressEndAngleSpeed, t);

			effect->SetAlphaDirect(static_cast<int>(alpha));
			effect->SetScale(item.checkScale);
			effect->SetAngleSpeed(angleSpeed);

			if (item.stampMotion.IsFinished())
			{
				item.stampPhase = StampAnimationPhase::FadeOut;

				item.stampMotion.Start(
					0.0f,
					1.0f,
					StampFadeDuration,
					FadeEasing);

				// 演出用スタンプの回転角度を引き継ぐ
				if (completedStamp)
					completedStamp->SetAngle(effect->GetAngle());
			}
		}

		// =================================
		// ② 演出用スタンプ消失
		// =================================
		else if (item.stampPhase == StampAnimationPhase::FadeOut)
		{
			const float alpha =
				Lerp(FadeStartAlpha, FadeEndAlpha, t);

			item.checkScale =
				Lerp(FadeStartScale, FadeEndScale, t);

			const float angleSpeed =
				Lerp(FadeStartAngleSpeed, FadeEndAngleSpeed, t);

			effect->SetAlphaDirect(static_cast<int>(alpha));
			effect->SetScale(item.checkScale);
			effect->SetAngleSpeed(angleSpeed);

			// 完了スタンプを徐々に表示
			if (completedStamp)
			{
				completedStamp->SetAlphaDirect(
					static_cast<int>(Lerp(0.0f, 255.0f, t)));
			}

			// 演出終了
			if (item.stampMotion.IsFinished())
			{
				effect->SetAlphaDirect(static_cast<int>(FadeEndAlpha));
				effect->SetAngleSpeed(0.0f);
				effect->SetFalse();


				item.stampPhase = StampAnimationPhase::Finished;
			}
		}
	}
	// =================================
	// 全項目達成 ＆ 全スタンプ演出終了
	// =================================
	bool allCompleted = !items.empty();

	for (const auto& item : items)
	{
		if (item.stampPhase != StampAnimationPhase::Finished)
		{
			allCompleted = false;
			break;
		}
	}

	if (allCompleted && !items.empty())
	{
		SetVisible(false);

		if (auto* list = UIM().GetUI(L"TutorialList"))
			list->Hide();
	}

	if (IsAllStampFinished())
		SetVisible(false);
}

void TutorialChecklist::AddItem(
	const std::wstring& text,
	DxPlus::Vec2 stampPosition)
{
	ChecklistItem item;

	item.text = text;
	item.stampPosition = stampPosition;

	// ランダムなXオフセット
	item.stampPosition.x +=
		DxLib::GetRand(StampRandomOffsetX * 2) - StampRandomOffsetX;

	items.push_back(std::move(item));
}

void TutorialChecklist::CompleteItem(std::size_t index)
{
	if (index >= items.size()) return;

	auto& item = items[index];

	if (item.completed) return;
	item.completed = true;

	const DxPlus::Vec2 position =
		panelPosition + item.stampPosition;

	const std::wstring stampId =
		L"TutorialStamp" + std::to_wstring(index);

	const std::wstring effectId =
		L"TutorialStampEffect" + std::to_wstring(index);

	// =================================
	// ① 完了スタンプ（残しておくUI）
	// =================================

	auto* stamp = UIM().CreateImageUI(
		stampId,
		ResourceKeys::TutorialStamp,
		position,
		DxPlus::Vec2(0.0f, 0.0f));

	if (stamp)
	{
		stamp->SetAlphaDirect(0);
		stamp->SetScale(1.0f);
		stamp->SetAngle(0.0f);
		stamp->SetAngleSpeed(0.0f);

		if (isVisible)
			stamp->SetVisible(true);
		else
			stamp->SetFalse();

		stampIds.push_back(stampId);
	}

	// =================================
	// ② 押印演出UI
	// =================================

	auto* effect = UIM().CreateImageUI(
		effectId,
		ResourceKeys::TutorialStamp,
		position,
		DxPlus::Vec2(0.0f, 0.0f));

	if (!effect)
	{
		// 演出UIを生成できなかった場合
		if (stamp)
			stamp->SetAlphaDirect(255);

		item.stampPhase = StampAnimationPhase::Finished;
		return;
	}

	effect->SetAlphaDirect(static_cast<int>(PressStartAlpha));
	effect->SetScale(PressStartScale);
	effect->SetAngle(EffectStartAngle + DxLib::GetRand(PressStartRandamAngle));
	effect->SetAngleSpeed(PressStartAngleSpeed);

	if (isVisible)
		effect->SetVisible(true);
	else
		effect->SetFalse();

	stampEffectIds.push_back(effectId);

	// =================================
	// 押印アニメーション開始
	// =================================

	item.checkScale = PressStartScale;
	item.stampPhase = StampAnimationPhase::Press;

	item.stampMotion.Start(
		0.0f,
		1.0f,
		StampPressDuration,
		PressEasing);
}

void TutorialChecklist::SetVisible(bool visible)
{
	if (isVisible == visible) return;

	isVisible = visible;

	// =================================
	// チェックリスト本体
	// =================================

	if (auto* list = UIM().GetUI(L"TutorialList"))
	{
		if (!isVisible)
			list->Hide();
		else
			list->FadeIn(0.5f);
	}

	// =================================
	// 完了スタンプ
	// =================================

	for (const auto& stampId : stampIds)
	{
		if (auto* stamp = UIM().GetUI(stampId))
		{
			if (!isVisible)
				stamp->SetFalse();
			else
				stamp->SetVisible(true);
		}
	}

	// =================================
	// 演出用スタンプ
	// =================================

	for (std::size_t i = 0; i < items.size(); ++i)
	{
		const std::wstring effectId =
			L"TutorialStampEffect" + std::to_wstring(i);

		if (auto* effect = UIM().GetUI(effectId))
		{
			if (!isVisible)
			{
				effect->SetFalse();
				effect->SetAngleSpeed(0.0f);
			}
			else if (items[i].stampPhase == StampAnimationPhase::Press ||
				items[i].stampPhase == StampAnimationPhase::FadeOut)
			{
				effect->SetVisible(true);
			}
		}
	}
}
bool TutorialChecklist::IsAllStampFinished() const
{
	if (items.empty()) return false;

	for (const auto& item : items)
	{
		if (item.stampPhase != StampAnimationPhase::Finished)
			return false;
	}

	return true;
}