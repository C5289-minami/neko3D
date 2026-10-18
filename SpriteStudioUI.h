#pragma once
#include <memory>
#include <string>
#include "UIBase.h"
#include "SSPlayer/SS6Player.h"

class SpriteStudioUI final : public UIBase
{
public:
    SpriteStudioUI(const std::wstring& key, DxPlus::Vec2 pos,
        DxPlus::Vec2 dir = DxPlus::Vec2(0, 0), float motionDuration = 0.0f,
        const std::string& animationName = "");
    SpriteStudioUI(const std::wstring& key, ss::Player* player, DxPlus::Vec2 pos,
        DxPlus::Vec2 dir = DxPlus::Vec2(0, 0), float motionDuration = 0.0f);

    void Update(float deltaTime) override;
    void Draw() override;
    void SetScale(float targetScale) override;
    void SetProgress(float progress);
    void PlayAnimation(const std::string& animationName = "");

private:
    void StartAnimation(const std::string& animationName);

    std::unique_ptr<ss::Player> ownedPlayer;
    ss::Player* player{};
    float progress{};
    bool progressControlled{};
};
