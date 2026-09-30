#pragma once
#include <algorithm>
#include <functional>
#include <string>
#include "DxPlus/DxPlus.h"
#include "ResourceManager.h"

class Button
{
public:
    Button(const DxPlus::Vec2& pos, float w, float h, std::function<void()> callback)
        : position(pos), onClick(std::move(callback)), center(w / 2.0f, h / 2.0f)
    {
    }

    Button(const DxPlus::Vec2& pos, const std::wstring& key, std::function<void()> callback)
        : position(pos), spriteKey(key), onClick(std::move(callback))
    {
        UpdateSpriteSize();
    }

    enum class TriggerMode
    {
        Down,
        Up,
        Hold
    };

    void SetTriggerMode(TriggerMode mode) { triggerMode = mode; }

    void Update(const DxPlus::Vec2& mousePos, bool mousePush)
    {
        if (!isDraw) return;

        const float halfWidth = center.x * scale;
        const float halfHeight = center.y * scale;
        isHovered = mousePos.x >= position.x - halfWidth && mousePos.x <= position.x + halfWidth &&
            mousePos.y >= position.y - halfHeight && mousePos.y <= position.y + halfHeight;

        const bool justPressed = mousePush && !prevMousePush;
        const bool justReleased = !mousePush && prevMousePush;
        if (justPressed && isHovered) isPressedOnButton = true;

        bool triggered = false;
        switch (triggerMode)
        {
        case TriggerMode::Down:
            triggered = isHovered && justPressed;
            break;
        case TriggerMode::Up:
            triggered = isHovered && isPressedOnButton && justReleased;
            break;
        case TriggerMode::Hold:
            triggered = isHovered && mousePush;
            break;
        }

        if (justReleased || !isHovered) isPressedOnButton = false;
        prevMousePush = mousePush;

        const bool hasHoverSprite = !hoverSpriteKey.empty() && RM().GetSprite(hoverSpriteKey);
        alpha = baseAlpha * ((isHovered && !hasHoverSprite) ? 128 : 255) / 255;
        if (triggered && onClick) onClick();
    }

    void Draw() const
    {
        if (!isDraw) return;

        const auto* sprite = isHovered && !hoverSpriteKey.empty() ? RM().GetSprite(hoverSpriteKey) : nullptr;
        if (!sprite) sprite = RM().GetSprite(spriteKey);
        if (!sprite) return;

        DxLib::SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        sprite->Draw(position, DxPlus::Vec2(scale, scale));
        DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

    void DebugDraw() const
    {
        const float halfWidth = center.x * scale;
        const float halfHeight = center.y * scale;
        DxLib::DrawBoxAA(position.x - halfWidth, position.y - halfHeight,
            position.x + halfWidth, position.y + halfHeight,
            DxLib::GetColor(255, 0, 0), true, 2.0f);
    }

    void SetDraw(bool draw) { isDraw = draw; }
    void SetAlpha(int value) { baseAlpha = std::clamp(value, 0, 255); }
    void SetHoverSprite(const std::wstring& key) { hoverSpriteKey = key; }
    void SetScale(float value) { scale = value; }
    void SetPosition(const DxPlus::Vec2& pos) { position = pos; }

private:
    void UpdateSpriteSize()
    {
        const auto* sprite = RM().GetSprite(spriteKey);
        if (!sprite) return;

        float width = 0.0f;
        float height = 0.0f;
        DxLib::GetGraphSizeF(sprite->GetID(), &width, &height);
        center = DxPlus::Vec2(width / 2.0f, height / 2.0f);
    }

    DxPlus::Vec2 position{};
    DxPlus::Vec2 center{};
    float scale{ 1.0f };
    std::wstring spriteKey;
    std::wstring hoverSpriteKey;
    int alpha{ 255 };
    int baseAlpha{ 255 };
    bool isHovered{ false };
    std::function<void()> onClick{};
    bool isDraw{ true };
    TriggerMode triggerMode{ TriggerMode::Down };
    bool prevMousePush{ false };
    bool isPressedOnButton{ false };
};