#include "StatusUI.h"
#include <algorithm>
#include "Player.h"
#include "ResourceKeys.h"
#include "ResourceManager.h"
#include "DxPlus/DxPlus.h"

namespace
{
	std::wstring selectedPlayerBackground;

	std::wstring PlayerHeartKey(int index)
	{
		return L"PlayerHeart" + std::to_wstring(index);
	}

	const std::wstring StatusKeys[] =
	{
		L"playerULTGaugeBackMike",
		L"playerULTGaugeBackSabi",
		L"playerULTGaugeBackKuro",
		L"PlayerUltGauge",
		L"BossHealthGauge",
		L"BossDownGauge"
	};
}

void StatusUI::InitPlayer(const std::string& characterName)
{
	const DxPlus::Vec2 backgroundPosition(
		DxPlus::CLIENT_WIDTH * 0.5f,
		DxPlus::CLIENT_HEIGHT * 0.88f);

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

	selectedPlayerBackground = selectedBackground;

	for (const auto* key : {
		L"playerULTGaugeBackMike",
		L"playerULTGaugeBackSabi",
		L"playerULTGaugeBackKuro" })
	{
		if (auto* ui = UIM().GetUI(key))
			ui->SetFalse();
	}

	UIM().CreateSpriteStudioUI(
		L"PlayerUltGauge",
		ResourceKeys::SpriteStudio_PlayerUltGauge,
		DxPlus::Vec2(DxPlus::CLIENT_WIDTH * 0.5f + 80.0f,
			DxPlus::CLIENT_HEIGHT * 0.537f),
		"playerult");

	UIM().GetUI(L"PlayerUltGauge")->SetFalse();

	const auto* background = RM().GetSprite(ResourceKeys::MikeStatusBack);
	if (!background) return;

	float scale = 0.7f;

	float backgroundWidth = 0.0f;
	float backgroundHeight = 0.0f;
	DxLib::GetGraphSizeF(background->GetID(), &backgroundWidth, &backgroundHeight);


	const float left = backgroundPosition.x - backgroundWidth * 0.5f;
	const float rowStart = left + backgroundWidth * 0.29f; // HPのStartPos
	const float slotWidth = 76.0f * scale * 1.25f; // 並べる間隔
	const float rowY = backgroundPosition.y + backgroundHeight * 0.4f;

	for (int i = 0; i < Player::MaxHealth; ++i)
	{
		const DxPlus::Vec2 position(
			rowStart + (i + 0.5f) * slotWidth,
			rowY
		);

		const auto key = PlayerHeartKey(i);
		auto* heart = UIM().GetUI(key);

		if (!heart)
			heart = UIM().CreateImageUI(key, ResourceKeys::PlayerHeart, position);

		heart->SetFalse();
		heart->SetCenter({ 0.5f,0.5f });
		heart->SetScale(scale);
	}
}

void StatusUI::InitBoss()
{
	const DxPlus::Vec2 backgroundPosition(
		DxPlus::CLIENT_WIDTH * 0.5f + 10,
		DxPlus::CLIENT_HEIGHT * 0.5f);

	UIM().CreateSpriteStudioUI(
		L"BossHealthGauge",
		ResourceKeys::SpriteStudio_BossHealthGauge,
		backgroundPosition,
		"Boss_hp");

	UIM().CreateSpriteStudioUI(
		L"BossDownGauge",
		ResourceKeys::SpriteStudio_BossDownGauge,
		backgroundPosition,
		"Boss_down");

	UIM().GetUI(L"BossHealthGauge")->SetFalse();
	UIM().GetUI(L"BossDownGauge")->SetFalse();
}

void StatusUI::SyncStatusBar(const std::wstring& key, float progress)
{
	auto* ui = UIM().GetSpriteStudioUI(key);

	if (!ui) return;

	ui->SetProgress(std::clamp(progress, 0.0f, 1.0f));
}

void StatusUI::SyncPlayerHealth(int health)
{
	health = std::clamp(health, 0, Player::MaxHealth);

	for (int i = 0; i < Player::MaxHealth; ++i)
	{
		if (auto* heart = UIM().GetUI(PlayerHeartKey(i)))
		{
			if (i < health)
				heart->SetTrue();
			else
				heart->SetFalse();
		}
	}
}
void StatusUI::Open()
{
	const std::wstring keys[] =
	{
		L"playerULTGaugeBackMike",
		L"playerULTGaugeBackSabi",
		L"playerULTGaugeBackKuro",
		L"PlayerUltGauge",
		//L"BossStatusBackground",
		L"BossHealthGauge",
		L"BossDownGauge"
	};

	for (const auto& key : keys)
	{
		if (auto* ui = UIM().GetUI(key))
		{
			if (key == L"PlayerUltGauge" ||
				key == L"BossHealthGauge" ||
				key == L"BossDownGauge" ||
				key == selectedPlayerBackground)
			{
				ui->SetTrue();
			}
		}
	}

	for (int i = 0; i < Player::MaxHealth; ++i)
	{
		if (auto* heart = UIM().GetUI(PlayerHeartKey(i)))
			heart->SetTrue();
	}
}

void StatusUI::Hide()
{
	const std::wstring keys[] =
	{
		L"playerULTGaugeBackMike",
		L"playerULTGaugeBackSabi",
		L"playerULTGaugeBackKuro",
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
