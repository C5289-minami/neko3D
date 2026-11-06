#include "StatusUI.h"
#include <algorithm>
#include "Player.h"
#include "ResourceKeys.h"
#include "ResourceManager.h"
#include "DxPlus/DxPlus.h"

namespace
{
    const std::wstring StatusKeys[] =
    {
        L"PlayerStatusBackground",
        L"PlayerUltGauge",
        L"BossStatusBackground",
        L"BossHealthGauge",
        L"BossDownGauge"
    };

    std::wstring PlayerHeartKey(int index)
    {
        return L"PlayerHeart" + std::to_wstring(index);
    }
}

void StatusUI::InitPlayer(const Player* player)
{
    const DxPlus::Vec2 backgroundPosition(DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.91f);

    UIM().CreateImageUI(
        L"PlayerStatusBackground",
        ResourceKeys::MikeStatusBack,
        backgroundPosition);

    UIM().CreateSpriteStudioUI(
        L"PlayerUltGauge",
        ResourceKeys::SpriteStudio_PlayerUltGauge,
        backgroundPosition + DxPlus::Vec2(0.0f, 10.0f));

    UIM().GetUI(L"PlayerStatusBackground")->SetFalse();
    UIM().GetUI(L"PlayerUltGauge")->SetFalse();

    const auto* background = RM().GetSprite(ResourceKeys::MikeStatusBack);
    if (!background) return;

    float width = 0.0f;
    float height = 0.0f;
    DxLib::GetGraphSizeF(background->GetID(), &width, &height);

    const float left = backgroundPosition.x - width * 0.5f;
    
    const float rowStart = left + width * 0.35f;
    
    const float slotWidth = width * 0.59f / Player::MaxHealth;
    const float rowY = backgroundPosition.y - height * 0.03f;
    
    constexpr float heartScale = 0.7f;

    for (int i = 0; i < Player::MaxHealth; ++i)
    {
        const DxPlus::Vec2 position(rowStart + (i + 0.5f) * slotWidth, rowY);
        const auto key = PlayerHeartKey(i);
        auto* heart = UIM().GetUI(key);
        if (!heart)
            heart = UIM().CreateImageUI(key, ResourceKeys::PlayerHeart, position);
        heart->SetScale(heartScale);
        heart->SetDisplayCondition([player, i]() { return !player || i < player->GetHealth(); });
        heart->SetFalse();
    }
}

void StatusUI::InitBoss()
{
    const DxPlus::Vec2 backgroundPosition(DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.08f);

    UIM().CreateImageUI(
        L"BossStatusBackground",
        ResourceKeys::BossStatusBack,
        backgroundPosition);

    UIM().CreateSpriteStudioUI(
        L"BossHealthGauge",
        ResourceKeys::SpriteStudio_BossHealthGauge,
        backgroundPosition + DxPlus::Vec2(0.0f, -10.0f));

    UIM().CreateSpriteStudioUI(
        L"BossDownGauge",
        ResourceKeys::SpriteStudio_BossDownGauge,
        backgroundPosition + DxPlus::Vec2(0.0f, 10.0f));

    UIM().GetUI(L"BossStatusBackground")->SetFalse();
    UIM().GetUI(L"BossHealthGauge")->SetFalse();
    UIM().GetUI(L"BossDownGauge")->SetFalse();
}
void StatusUI::SyncStatusBar(const std::wstring& key, float progress)
{
    auto* ui = UIM().GetSpriteStudioUI(key);

    if (!ui) return;

    ui->SetProgress(std::clamp(progress, 0.0f, 1.0f));
}

void StatusUI::Open()
{
    for (const auto& key : StatusKeys)
    {
        if (auto* ui = UIM().GetUI(key))
            ui->SetTrue();
    }

    for (int i = 0; i < Player::MaxHealth; ++i)
    {
        if (auto* heart = UIM().GetUI(PlayerHeartKey(i)))
            heart->SetTrue();
    }
}

void StatusUI::Hide()
{
    for (const auto& key : StatusKeys)
    {
        if (auto* ui = UIM().GetUI(key))
            ui->Hide();
    }

    for (int i = 0; i < Player::MaxHealth; ++i)
    {
        if (auto* heart = UIM().GetUI(PlayerHeartKey(i)))
            heart->SetFalse();
    }
}

void StatusUI::Remove()
{
    for (const auto& key : StatusKeys)
        UIM().RemoveUI(key);

    for (int i = 0; i < Player::MaxHealth; ++i)
        UIM().RemoveUI(PlayerHeartKey(i));
}
