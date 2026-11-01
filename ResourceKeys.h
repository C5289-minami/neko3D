// =============================
// Resources/ResourceKeys.h
// =============================
#pragma once


namespace ResourceKeys
{
	// ===== UI =====
	struct Button
	{
		const wchar_t* back;
		const wchar_t* hover;
	};
	inline constexpr const wchar_t* UI_Logo = L"UILogo";

	inline constexpr const Button Button_Red = { L"UIRedButton", L"UIRedHover" };
	inline constexpr const Button Button_Pink = { L"UIPinkButton", L"UIPinkHover" };
	inline constexpr const Button Button_Purple = { L"UIPurpleButton", L"UIPurpleHover" };
	inline constexpr const Button Button_Rose = { L"UIRoseButton", L"UIRoseHover" };


	// ===== Textures / Grids =====
	//inline constexpr const wchar_t* Texture_openPBR_shader1 = L"openPBR_shader1";
	//inline constexpr const wchar_t* Texture_petanurihatyottohuben = L"petanurihatyottohuben";

	// ===== Models =====
	inline constexpr const wchar_t* Model_PlayerMike = L"ModelPlayerMike";
	inline constexpr const wchar_t* Model_PlayerSabi = L"ModelPlayerSabi";
	inline constexpr const wchar_t* Model_PlayerKuro = L"ModelPlayerKuro";


	inline constexpr const wchar_t* Model_Boss = L"ModelBoss";
	inline constexpr const wchar_t* Model_Room = L"ModelRoom";
	inline constexpr const wchar_t* Model_Test = L"ModelTest";
	inline constexpr const wchar_t* Model_Basket = L"ModelBasket";
	// Boss Anim
	inline constexpr const wchar_t* Model_BossAnim_Idle = L"BossAnimIdle";
	inline constexpr const wchar_t* Model_BossAnim_Sippo = L"BossAnimSippo";

	// ===== Stage Models =====
	inline constexpr const wchar_t* Model_PillarL = L"hashira_l";
	inline constexpr const wchar_t* Model_PillarM = L"hashira_m";
	inline constexpr const wchar_t* Model_PillarS = L"hashira_s";
	inline constexpr const wchar_t* Model_StageFbx = L"Stage";
	inline constexpr const wchar_t* Model_StageAlpha = L"ModelStageAlpha";
	inline constexpr const wchar_t* Model_stand_l = L"stand_l";
	inline constexpr const wchar_t* Model_stand_m = L"stand_m";
	inline constexpr const wchar_t* Model_JumpStand = L"JumpStand";

	// ===== SpriteStudio =====
	inline constexpr const wchar_t* SpriteStudio_Gauge = L"Gauge";

	// ===== Effects =====
	inline constexpr const wchar_t* Effect_Test = L"EffectTest";

	// ===== Musics / Sounds =====
	inline constexpr const wchar_t* Sound_BossBite = L"SoundBossBite";

	// ===== Fonts =====
	inline constexpr const wchar_t* Font_Title = L"Bitcount Light";
	inline constexpr const wchar_t* Font_Game = L"WDXL Lubrifont JP N";
}
