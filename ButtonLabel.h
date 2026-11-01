#pragma once

#include <string>
#include "DxPlus/DxPlus.h"

class ButtonLabel
{
public:
    ButtonLabel() = default;

    void Draw(DxPlus::Vec2 position, float scale, int alpha) const;

    void SetText(const std::wstring& text) { text_ = text; }
    const std::wstring& GetText() const { return text_; }

    void SetFontHandle(int handle) { fontHandle_ = handle; }
    void SetColor(int color) { color_ = color; }
    void SetAlign(DxPlus::Text::TextAlign align) { align_ = align; }
    void SetOffset(DxPlus::Vec2 offset) { offset_ = offset; }
    void SetScale(float scale) { scale_ = scale; }

private:
    std::wstring text_;

    int fontHandle_{ -1 };
    unsigned int color_{ DxLib::GetColor(255, 255, 255) };

    DxPlus::Text::TextAlign align_{ DxPlus::Text::MIDDLE_CENTER };

    DxPlus::Vec2 offset_{};
    float scale_{ 1.0f };
};