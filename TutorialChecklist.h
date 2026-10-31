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

struct ChecklistItem
{
    std::wstring text;
    bool completed{ false };

    float checkScale{ 0.0f };

    Motion checkMotion;
};

class TutorialChecklist
{
public:
    void Init();
    void Update(float deltaTime);
    void Draw() const;

    void AddItem(const std::wstring& text);
    void CompleteItem(std::size_t index);

    void SetVisible(bool visible);

private:
    std::vector<ChecklistItem> items;

    Motion panelMotion;

    DxPlus::Vec2 position{ 30.0f, 100.0f };

    bool isVisible{ true };
};