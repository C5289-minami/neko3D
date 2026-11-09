
#include "StatusUI.h"
#include <algorithm>
#include "Player.h"
#include "ResourceKeys.h"
#include "ResourceManager.h"
#include "DxPlus/DxPlus.h"

namespace
{
	std::wstring selectedPlayerBackground;

	constexpr DxPlus::Vec2 StatusPlayerMotionDirection{
		0.0f,
		-1.0f
	};
	constexpr DxPlus::Vec2 StatusBossMotionDirection{
		0.0f,
		1.0f
	};

	constexpr float StatusMotionDuration = 1.0f;
	constexpr float HeartNormalSize = 0.7f;
	constexpr float HeartBigSize = HeartNormalSize * 1.2f;
	constexpr float HeartBigSizeChangeTime = 0.15f;			// 膨らむ時間やで
	constexpr float HeartShrinkSizeChangeTime = 0.25f;
	// ハートの大きさを元に戻す時間
	constexpr float HeartBigSizeChangeSpeed = 0.933333f;
	constexpr float HeartSmallSizeChangeSpeed = 3.36f;
	constexpr float HeartAnimationTotalTime = HeartBigSizeChangeTime + HeartShrinkSizeChangeTime;
	float heartEffectTimeLeft[Player::MaxHealth]{};
	int beforeHealth = 0;
	bool heartUiIsOpen = false;

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
		L"BossUIBack",
		L"BossHealthGauge",
		L"BossDownGauge",
		L"ULT_EButton"
	};
}

void StatusUI::InitPlayer(const std::string& characterName)
{
	heartUiIsOpen = false;
	const DxPlus::Vec2 backgroundPosition(
		DxPlus::CLIENT_WIDTH * 0.5f,
		DxPlus::CLIENT_HEIGHT * 0.88f);

	// プレイヤー背景
	auto* mike = UIM().GetUI(L"playerULTGaugeBackMike");
	if (!mike)
		mike = UIM().CreateImageUI(
			L"playerULTGaugeBackMike",
			ResourceKeys::PlayerStatus.mike,
			backgroundPosition,
			StatusPlayerMotionDirection,
			StatusMotionDuration);

	auto* sabi = UIM().GetUI(L"playerULTGaugeBackSabi");
	if (!sabi)
		sabi = UIM().CreateImageUI(
			L"playerULTGaugeBackSabi",
			ResourceKeys::PlayerStatus.sabi,
			backgroundPosition,
			StatusPlayerMotionDirection,
			StatusMotionDuration);

	auto* kuro = UIM().GetUI(L"playerULTGaugeBackKuro");
	if (!kuro)
		kuro = UIM().CreateImageUI(
			L"playerULTGaugeBackKuro",
			ResourceKeys::PlayerStatus.kuro,
			backgroundPosition,
			StatusPlayerMotionDirection,
			StatusMotionDuration);

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

	// ULTゲージ
	auto* ultGauge = UIM().GetUI(L"PlayerUltGauge");
	if (!ultGauge)
		ultGauge = UIM().CreateSpriteStudioUI(
			L"PlayerUltGauge",
			ResourceKeys::SpriteStudio_PlayerUltGauge,
			DxPlus::Vec2(
				DxPlus::CLIENT_WIDTH * 0.5f + 80.0f,
				DxPlus::CLIENT_HEIGHT * 0.537f
			),
			"playerult",
			StatusPlayerMotionDirection,
			StatusMotionDuration
		);

	if (ultGauge)
		ultGauge->SetFalse();

	// 背景画像サイズ取得
	const auto* background = RM().GetSprite(ResourceKeys::PlayerStatus.mike);
	const auto* heartSprite = RM().GetSprite(ResourceKeys::PlayerHeart);

	if (!background || !heartSprite) return;

	float backgroundWidth = 0.0f;
	float backgroundHeight = 0.0f;
	float heartWidth = 0.0f;
	float heartHeight = 0.0f;

	DxLib::GetGraphSizeF(
		background->GetID(), &backgroundWidth, &backgroundHeight);

	DxLib::GetGraphSizeF(
		heartSprite->GetID(), &heartWidth, &heartHeight);

	const float left = backgroundPosition.x - backgroundWidth * 0.5f;
	const float rowStart = left + backgroundWidth * 0.29f;
	const float slotWidth = heartWidth * HeartNormalSize * 1.25f;
	const float rowY = backgroundPosition.y + backgroundHeight * 0.4f;

	// HPハート
	for (int i = 0; i < Player::MaxHealth; ++i)
	{
		const DxPlus::Vec2 position(
			rowStart + (i + 0.5f) * slotWidth,
			rowY);

		const auto key = PlayerHeartKey(i);
		auto* heart = UIM().GetUI(key);

		if (!heart)
			heart = UIM().CreateImageUI(
				key,
				ResourceKeys::PlayerHeart,
				position);

		if (heart)
		{
			heart->SetFalse();
			heart->SetCenter({ 0.5f, 0.5f });
			heart->SetScale(HeartNormalSize);
		}
	}

	// ULTボタン
	auto* ultButton = UIM().GetUI(L"ULT_EButton");
	if (!ultButton)
		ultButton = UIM().CreateImageUI(
			L"ULT_EButton",
			ResourceKeys::Button_E.back,
			{
				left - 10.0f,
				backgroundPosition.y - backgroundHeight * 0.5f - 10.0f
			},
			StatusBossMotionDirection,
			StatusMotionDuration
		);
	ultButton->SetScale(0.7f);
	ultButton->SetFalse();
}

void StatusUI::InitBoss()
{
	const DxPlus::Vec2 backgroundPosition(
		DxPlus::CLIENT_WIDTH * 0.5f,
		DxPlus::CLIENT_HEIGHT * 0.12f
	);

	constexpr float GaugeOffsetX = 45.0f;
	constexpr float GaugeOffsetY = 20.0f;

	auto* bossBack =
		UIM().GetUI(L"BossUIBack");

	if (!bossBack)
	{
		bossBack = UIM().CreateImageUI(
			L"BossUIBack",
			ResourceKeys::BossStatusBack,
			backgroundPosition +
			DxPlus::Vec2{
				0.0f,
				-GaugeOffsetY
			},
			StatusBossMotionDirection,
			StatusMotionDuration
		);
	}

	if (bossBack)
	{
		bossBack->SetFalse();
	}

	// ボスHPゲージ
	auto* healthGauge =
		UIM().GetUI(L"BossHealthGauge");

	const DxPlus::Vec2 healthGaugePosition =
		backgroundPosition +
		DxPlus::Vec2{
			GaugeOffsetX + 19.0f,
			-GaugeOffsetY + 4.0f
	};

	if (!healthGauge)
	{
		healthGauge =
			UIM().CreateSpriteStudioUI(
				L"BossHealthGauge",
				ResourceKeys::SpriteStudio_BossHealthGauge,
				healthGaugePosition,
				"Boss_hp",
				StatusBossMotionDirection,
				StatusMotionDuration
			);
	}

	if (healthGauge)
	{
		healthGauge->SetFalse();
	}

	// ボスダウンゲージ
	auto* downGauge =
		UIM().GetUI(L"BossDownGauge");

	if (!downGauge)
	{
		downGauge =
			UIM().CreateSpriteStudioUI(
				L"BossDownGauge",
				ResourceKeys::SpriteStudio_BossDownGauge,
				backgroundPosition +
				DxPlus::Vec2{
					GaugeOffsetX,
					GaugeOffsetY
				},
				"Boss_down",
				StatusBossMotionDirection,
				StatusMotionDuration
			);
	}

	if (downGauge)
	{
		downGauge->SetFalse();
	}
}

void StatusUI::SyncStatusBar(const std::wstring& key, float progress)
{
	auto* ui = UIM().GetSpriteStudioUI(key);

	if (!ui) return;

	ui->SetProgress(std::clamp(progress, 0.0f, 1.0f));
}

void StatusUI::SyncPlayerHealth(int health, float deltaTime)
{
	if (!heartUiIsOpen) return;

	health = std::clamp(health, 0, Player::MaxHealth);

	for (int i = 0; i < Player::MaxHealth; ++i)
	{
		auto* heart = UIM().GetUI(PlayerHeartKey(i));
		if (!heart) continue;

		// HPが残っているハートは、通常の大きさで表示する
		if (i < health)
		{
			heartEffectTimeLeft[i] = 0.0f;
			heart->SetScale(HeartNormalSize);
			heart->SetTrue();
			continue;
		}

		float remainingTime = heartEffectTimeLeft[i];
		// 今回失ったハートは演出を始め、それ以外は残り時間を減らす
		if (i < beforeHealth)
		{
			remainingTime = HeartAnimationTotalTime;
		}
		else// すでに失ったハートは、残り時間を減らす
		{
			remainingTime = std::max(0.0f, remainingTime - deltaTime);
		}
		// 残り時間を更新
		heartEffectTimeLeft[i] = remainingTime;

		// 先に膨らませ、残り時間が少なくなったら縮める
		if (remainingTime > HeartShrinkSizeChangeTime)
		{

			const float bigSizeTime = HeartAnimationTotalTime - remainingTime;
			const float newSize = HeartNormalSize +
				HeartBigSizeChangeSpeed * bigSizeTime;
			heart->SetScale(newSize);
		}
		else if (remainingTime > 0.0f)
		{
			const float smallSizeTime = HeartShrinkSizeChangeTime - remainingTime;
			const float newSize = HeartBigSize -
				HeartSmallSizeChangeSpeed * smallSizeTime;
			heart->SetScale(newSize);
		}
		else
		{
			heart->SetFalse();
		}
	}

	beforeHealth = health;
}

bool StatusUI::IsPlayerHeartLossAnimating()
{
	if (!heartUiIsOpen) return false;

	for (float leftTime : heartEffectTimeLeft)
	{
		if (leftTime > 0.0f) return true;
	}
	return false;
}

void StatusUI::Open(int playerHealth)
{
	const std::wstring keys[] =
	{
		L"PlayerUltGauge",
		L"BossUIBack",
		L"BossHealthGauge",
		L"BossDownGauge",
		L"ULT_EButton"
	};

	// 選択したプレイヤー背景を表示する。
	if (auto* background =
		UIM().GetUI(selectedPlayerBackground))
	{
		background->Show();
	}

	for (const auto& key : keys)
	{
		if (auto* ui = UIM().GetUI(key))
		{
			ui->Show();
		}
	}

	// 開幕時は、現在のHPを被弾演出なしで表示する。
	std::fill_n(heartEffectTimeLeft, Player::MaxHealth, 0.0f);
	beforeHealth = std::clamp(playerHealth, 0, Player::MaxHealth);
	heartUiIsOpen = true;
	SyncPlayerHealth(playerHealth, 0.0f);
}

void StatusUI::Hide()
{
	heartUiIsOpen = false;
	const std::wstring keys[] =
	{
		L"playerULTGaugeBackMike",
		L"playerULTGaugeBackSabi",
		L"playerULTGaugeBackKuro",
		L"PlayerUltGauge",
		L"BossUIBack",
		L"BossHealthGauge",
		L"BossDownGauge",
		L"ULT_EButton"
	};

	for (const auto& key : keys)
	{
		if (auto* ui = UIM().GetUI(key))
		{
			ui->Hide();
		}
	}

	for (int i = 0; i < Player::MaxHealth; ++i)
	{
		if (auto* heart =
			UIM().GetUI(PlayerHeartKey(i)))
		{
			heart->SetFalse();
		}
	}
}

void StatusUI::Remove()
{
	heartUiIsOpen = false;
	for (const auto& key : StatusKeys)
		UIM().RemoveUI(key);

	for (int i = 0; i < Player::MaxHealth; ++i)
		UIM().RemoveUI(PlayerHeartKey(i));
}
