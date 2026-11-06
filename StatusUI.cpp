#include "StatusUI.h"
#include <algorithm>
#include "ResourceKeys.h"
#include "DxPlus/DxPlus.h"

void StatusUI::InitPlayer(const std::string& characterName)
{
    const DxPlus::Vec2 backgroundPosition(
        DxPlus::CLIENT_WIDTH * 0.5f,
        DxPlus::CLIENT_HEIGHT * 0.8f);

    UIM().CreateImageUI(
        L"playerULTGaugeBackMike",
        ResourceKeys::MikeStatusBack,
        backgroundPosition
        );
    UIM().CreateImageUI(
        L"playerULTGaugeBackSabi",
        ResourceKeys::SabiStatusBack,
        backgroundPosition
        );
    UIM().CreateImageUI(
        L"playerULTGaugeBackKuro",
        ResourceKeys::KuroStatusBack,
        backgroundPosition
        );

    const wchar_t* selectedBackground =
        characterName == "ぎし" ? L"playerULTGaugeBackSabi" :
        characterName == "ゆう" ? L"playerULTGaugeBackKuro" :
        L"playerULTGaugeBackMike";

    for (const auto* key : {
        L"playerULTGaugeBackMike",
        L"playerULTGaugeBackSabi",
        L"playerULTGaugeBackKuro" })
    {
        if (auto* ui = UIM().GetUI(key))
            ui->SetFalse();
    }

    if (auto* ui = UIM().GetUI(selectedBackground))
        ui->SetTrue();

    UIM().CreateSpriteStudioUI(
        L"PlayerUltGauge",
        ResourceKeys::SpriteStudio_PlayerUltGauge,
        DxPlus::Vec2(DxPlus::CLIENT_WIDTH * 0.5f,
            DxPlus::CLIENT_HEIGHT * 0.5f),
        "playerult");

    UIM().GetUI(L"PlayerUltGauge")->SetFalse();
}

void StatusUI::InitBoss()
{
    const DxPlus::Vec2 backgroundPosition(
        DxPlus::CLIENT_WIDTH * 0.5f,
        DxPlus::CLIENT_HEIGHT * 0.5f);

    //UIM().CreateSpriteStudioUI(
    //    L"BossHealthGauge",
    //    ResourceKeys::SpriteStudio_BossHealthGauge,
    //    backgroundPosition,
    //    "Boss_hp");

    UIM().CreateSpriteStudioUI(
        L"BossDownGauge",
        ResourceKeys::SpriteStudio_BossDownGauge,
        DxPlus::Vec2(DxPlus::CLIENT_WIDTH * 0.5f,
            DxPlus::CLIENT_HEIGHT * 0.5f),
        "Boss_down");

   // UIM().GetUI(L"BossHealthGauge")->SetFalse();
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
    const std::wstring keys[] =
    {
        //L"PlayerStatusBackground",
       // L"PlayerHealthGauge",
        L"PlayerUltGauge",
        //L"BossStatusBackground",
        L"BossHealthGauge",
        L"BossDownGauge"
    };

    for (const auto& key : keys)
    {
        if (auto* ui = UIM().GetUI(key))
            ui->SetTrue();
    }
}

void StatusUI::Hide()
{
    const std::wstring keys[] =
    {
        //L"PlayerStatusBackground",
       // L"PlayerHealthGauge",
        L"PlayerUltGauge",
        //L"BossStatusBackground",
        L"BossHealthGauge",
        L"BossDownGauge"
    };

    for (const auto& key : keys)
    {
        if (auto* ui = UIM().GetUI(key))
            ui->Hide();
    }
}