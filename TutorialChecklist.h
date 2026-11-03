#pragma once

#include <string>
#include <vector>
#include "DxPlus/DxPlus.h"
#include "Motion.h"

/*
*   checklist.AddItem(L"移動する");
*   checklist.AddItem(L"ジャンプする");
*   checklist.AddItem(L"二段ジャンプする");
*   checklist.AddItem(L"柱を登る");
*   checklist.AddItem(L"攻撃を回避する");
*/

enum class StampAnimationPhase
{
    None,
    Press,
    FadeOut,
    Finished
};

struct ChecklistItem
{
    std::wstring text;
    DxPlus::Vec2 stampPosition{};
    bool completed{ false };

    float checkScale{ 5.0f };

    Motion stampMotion{};
    StampAnimationPhase stampPhase{ StampAnimationPhase::None };
};

class TutorialChecklist
{
public:
    void Init();
    void Update(float deltaTime);

    void AddItem(const std::wstring& text, DxPlus::Vec2 stampPosition = {});
    void CompleteItem(std::size_t index);
    bool IsAllStampFinished() const;

    void SetVisible(bool visible);

private:
    std::vector<ChecklistItem> items;
    std::vector<std::wstring> stampIds;
    std::vector<std::wstring> stampEffectIds;

	DxPlus::Vec2 panelPosition{ 0.0f, 0.0f };
	bool isVisible{ true };
};