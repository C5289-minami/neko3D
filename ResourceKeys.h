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
  
	//Tutorial
	inline constexpr const wchar_t* TutorialBack = L"UITutorialBack";
	inline constexpr const wchar_t* TutorialStamp = L"UITutorialStamp";
	
	// status
	inline constexpr const wchar_t* BossStatusBack = L"UIBossStatusBack";
	inline constexpr const wchar_t* MikeStatusBack = L"UIMikeStatusBack";

	// result
	inline constexpr const wchar_t* ClearResult = L"UIClearResult";
	inline constexpr const wchar_t* GameOverResult = L"UIGameOverResult";
	// select
	inline constexpr const wchar_t* SelectMike = L"UISelectMike";
	inline constexpr const wchar_t* SelectSabi = L"UISelectSabi";
	inline constexpr const wchar_t* SelectKuro = L"UISelectKuro";

	// ===== Models =====
	inline constexpr const wchar_t* Model_PlayerMike = L"ModelPlayerMike";
	inline constexpr const wchar_t* Model_PlayerSabi = L"ModelPlayerSabi";
	inline constexpr const wchar_t* Model_PlayerKuro = L"ModelPlayerKuro";
	// Player Anim
	inline constexpr const wchar_t* Model_PlayerAnim_Idle = L"PlayerAnimIdle";
	inline constexpr const wchar_t* Model_PlayerAnim_Walk = L"PlayerAnimWalk";
	inline constexpr const wchar_t* Model_PlayerAnim_Jump = L"PlayerAnimJump";
	inline constexpr const wchar_t* Model_PlayerAnim_Dodge = L"PlayerAnimDodge";
	inline constexpr const wchar_t* Model_PlayerAnim_Attack = L"PlayerAnimAttack";
	inline constexpr const wchar_t* Model_PlayerAnim_Ult = L"PlayerAnimUlt";



	inline constexpr const wchar_t* Model_Boss = L"ModelBoss";
	// Boss Anim
	inline constexpr const wchar_t* Model_BossAnim_Idle = L"BossAnimIdle";
	inline constexpr const wchar_t* Model_BossAnim_Sippo = L"BossAnimSippo";
	inline constexpr const wchar_t* Model_BossAnim_Attck_Hit = L"BossAnimAtkHit";
	inline constexpr const wchar_t* Model_BossAnim_Down = L"BossAnimDown";
	inline constexpr const wchar_t* Model_BossAnim_Die = L"BossAnimDie";
	inline constexpr const wchar_t* Model_BossAnim_Nekodamashi = L"BossAnimNekodamashi";

	// ===== Stage Models =====
	inline constexpr const wchar_t* Model_PillarL = L"hashira_l";
	inline constexpr const wchar_t* Model_PillarM = L"hashira_m";
	inline constexpr const wchar_t* Model_PillarS = L"hashira_s";
	inline constexpr const wchar_t* Model_Stage = L"Stage";
	inline constexpr const wchar_t* Model_stand_l = L"stand_l";
	inline constexpr const wchar_t* Model_stand_m = L"stand_m";
	inline constexpr const wchar_t* Model_stand_s = L"stand_s";
	inline constexpr const wchar_t* Model_JumpStand = L"JumpStand";
	// Room
	inline constexpr const wchar_t* Model_Room = L"ModelRoom";
	inline constexpr const wchar_t* Model_Basket = L"ModelBasket";
	inline constexpr const wchar_t* Model_Chair = L"ModelChair";

	// ===== SpriteStudio =====
	inline constexpr const wchar_t* SpriteStudio_Gauge = L"Gauge";
	inline constexpr const wchar_t* SpriteStudio_PlayerHealthGauge = L"PlayerHealthGauge";
	inline constexpr const wchar_t* SpriteStudio_PlayerUltGauge = L"PlayerUltGauge";
	inline constexpr const wchar_t* SpriteStudio_BossHealthGauge = L"BossHealthGauge";
	inline constexpr const wchar_t* SpriteStudio_BossDownGauge = L"BossDownGauge";


	// ===== Effects =====
	inline constexpr const wchar_t* Effect_Test = L"EffectTest";
	inline constexpr const wchar_t* Effect_Impact = L"EffectImpact";
	inline constexpr const wchar_t* Effect_PlayerAttack = L"EffectAttack";
	inline constexpr const wchar_t* Effect_Kedama = L"EffectKedama";

	// ===== Musics / Sounds =====
	inline constexpr const wchar_t* Music_Title = L"MusicTitle";
	inline constexpr const wchar_t* Sound_MenuConfirm = L"SoundMenuConfirm";

	// ===== Fonts =====
	inline constexpr const wchar_t* Font_Title = L"Bitcount Light";
	inline constexpr const wchar_t* Font_Game = L"WDXL Lubrifont JP N";
}
