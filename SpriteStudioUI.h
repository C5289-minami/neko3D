#pragma once
#include "UIBase.h"
#include "SSPlayer/SS6Player.h"

class SpriteStudioUI final : public UIBase
{
public:
    SpriteStudioUI(const std::wstring& key, ss::Player* player, DxPlus::Vec2 pos,
        DxPlus::Vec2 dir = DxPlus::Vec2(0, 0), float motionDuration = 0.0f);

    void Update(float deltaTime) override;
    void Draw() override;
    void SetScale(float targetScale) override;

private:
    ss::Player* player{};
};
