#include "StatusUI.h"
#include <algorithm>
#include "ResourceKeys.h"
#include "DxPlus/DxPlus.h"

void StatusUI::InitPlayer()
{
    const DxPlus::Vec2 backgroundPosition(DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.9f);

    UIM().CreateImageUI(
        L"PlayerStatusBackground",
        ResourceKeys::MikeStatusBack,
        backgroundPosition);

    UIM().CreateSpriteStudioUI(
        L"PlayerHealthGauge",
        ResourceKeys::SpriteStudio_PlayerHealthGauge,
        backgroundPosition + DxPlus::Vec2(0.0f, -10.0f));

    UIM().CreateSpriteStudioUI(
        L"PlayerUltGauge",
        ResourceKeys::SpriteStudio_PlayerUltGauge,
        backgroundPosition + DxPlus::Vec2(0.0f, 10.0f));
}

void StatusUI::InitBoss()
{
    const DxPlus::Vec2 backgroundPosition(DxPlus::CLIENT_WIDTH * 0.5f, DxPlus::CLIENT_HEIGHT * 0.1f);

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
}

void StatusUI::SyncStatusBar(const std::wstring& key, float progress)
{
    auto* ui = UIM().GetSpriteStudioUI(key);

    if (!ui) return;

    ui->SetProgress(std::clamp(progress, 0.0f, 1.0f));
}