#include "TutorialChecklist.h"

#include <algorithm>

namespace
{
    constexpr int PanelWidth = 280;
    constexpr int HeaderHeight = 40;
    constexpr int ItemHeight = 34;
    constexpr int Padding = 12;
}

void TutorialChecklist::Init()
{
    items.clear();

    isVisible = false;
    panelMotion.Reset();
}

void TutorialChecklist::Update(float deltaTime)
{
    if (!isVisible) return;

    // パネル全体のアニメーション
    panelMotion.Update(deltaTime);

    // 各項目のアニメーション
    for (auto& item : items)
    {
        item.checkMotion.Update(deltaTime);

        item.checkScale = std::clamp(
            item.checkMotion.GetValue(),
            0.0f,
            1.0f
        );
    }
}

void TutorialChecklist::Draw() const
{
    if (!isVisible || items.empty()) return;

    const float panelRate = panelMotion.GetValue();

    // パネルの位置と透明度
    const int x = static_cast<int>(
        position.x - 25.0f * (1.0f - panelRate)
        );

    const int y = static_cast<int>(position.y);

    const int alpha = static_cast<int>(220.0f * panelRate);

    const int panelHeight =
        HeaderHeight +
        static_cast<int>(items.size()) * ItemHeight +
        Padding;

    // アルファブレンド開始
    DxLib::SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

    // 背景
    DxLib::DrawBox(
        x, y,
        x + PanelWidth,
        y + panelHeight,
        DxLib::GetColor(25, 25, 35),
        TRUE
    );

    // 枠線
    DxLib::DrawBox(
        x, y,
        x + PanelWidth,
        y + panelHeight,
        DxLib::GetColor(100, 100, 120),
        FALSE
    );

    // タイトル
    DxLib::DrawString(
        x + Padding,
        y + 10,
        L"チュートリアル",
        DxLib::GetColor(255, 255, 255)
    );

    // 各項目
    for (std::size_t i = 0; i < items.size(); ++i)
    {
        const auto& item = items[i];

        const int itemX = x + Padding;
        const int itemY =
            y + HeaderHeight + static_cast<int>(i) * ItemHeight;

        // チェックボックス
        constexpr int CheckSize = 20;

        DxLib::DrawBox(
            itemX,
            itemY,
            itemX + CheckSize,
            itemY + CheckSize,
            DxLib::GetColor(180, 180, 190),
            FALSE
        );

        // チェックマーク
        if (item.checkScale > 0.0f)
        {
            const int centerX = itemX + CheckSize / 2;
            const int centerY = itemY + CheckSize / 2;

            const float scale = item.checkScale;

            // 3点を中心から拡大
            const int p1X = centerX + static_cast<int>(-6 * scale);
            const int p1Y = centerY;

            const int p2X = centerX + static_cast<int>(-1 * scale);
            const int p2Y = centerY + static_cast<int>(5 * scale);

            const int p3X = centerX + static_cast<int>(7 * scale);
            const int p3Y = centerY - static_cast<int>(6 * scale);

            DxLib::DrawLine(
                p1X, p1Y,
                p2X, p2Y,
                DxLib::GetColor(100, 255, 120)
            );

            DxLib::DrawLine(
                p2X, p2Y,
                p3X, p3Y,
                DxLib::GetColor(100, 255, 120)
            );
        }

        // 文字色
        const unsigned int textColor = item.completed
            ? DxLib::GetColor(130, 145, 150)
            : DxLib::GetColor(255, 255, 255);

        DxLib::DrawString(
            itemX + 30,
            itemY + 2,
            item.text.c_str(),
            textColor
        );
    }

    // アルファブレンド終了
    DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void TutorialChecklist::AddItem(const std::wstring& text)
{
    ChecklistItem item;
    item.text = text;

    items.push_back(std::move(item));
}

void TutorialChecklist::CompleteItem(std::size_t index)
{
    if (index >= items.size()) return;

    auto& item = items[index];

    if (item.completed) return;

    item.completed = true;

    item.checkMotion.Start(
        0.0f,
        1.0f,
        0.25f,
        Motion::Easing::EaseOut
    );
}

void TutorialChecklist::SetVisible(bool visible)
{
    if (isVisible == visible) return;

    isVisible = visible;

    if (visible)
    {
        // 左からスライドして表示
        panelMotion.Start(
            0.0f,
            1.0f,
            0.3f,
            Motion::Easing::EaseOut
        );
    }
    else
    {
        panelMotion.Reset();
    }
}