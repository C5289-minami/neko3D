#include "ButtonLabel.h"



void ButtonLabel::Draw(DxPlus::Vec2 position, float scale, int alpha) const
{
    if (text_.empty()) return;

    position = position + offset_;

    const int r = (color_ >> 16) & 0xFF;
    const int g = (color_ >> 8) & 0xFF;
    const int b = color_ & 0xFF;

    DxLib::SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

    DxPlus::Text::DrawString(
        text_.c_str(),
        position,
        DxLib::GetColor(r, g, b),
        align_,
        { scale * scale_, scale * scale_ },
        0.0,
        fontHandle_
    );

    DxLib::SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}