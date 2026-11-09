// =============================
// Resources/ResourceKeys.h
// =============================
#pragma once

namespace ResourceKeys
{
    struct Button
    {
        const wchar_t* back;
        const wchar_t* hover;
    };

    struct Players
    {
        const wchar_t* mike;
        const wchar_t* sabi;
        const wchar_t* kuro;
    };

    struct PlayerHands
    {
        const Players normal;
        const Players pressed;
	};

    // UI
    inline constexpr const wchar_t* UI_Logo = L"UILogo";
    inline constexpr const wchar_t* UI_CreditImage = L"UICreditImage";

    inline constexpr Button Button_Red{ L"UIRedButton", L"UIRedHover" };
    inline constexpr Button Button_Pink{ L"UIPinkButton", L"UIPinkHover" };
    inline constexpr Button Button_Purple{ L"UIPurpleButton", L"UIPurpleHover" };
    inline constexpr Button Button_Rose{ L"UIRoseButton", L"UIRoseHover" };
    inline constexpr Button Button_E{ L"EButton", L"EButtonHover" };

    inline constexpr PlayerHands PlayerHand{
        Players{ L"UIPlayerMikeHand", L"UIPlayerSabiHand", L"UIPlayerKuroHand" },
        Players{ L"UIPlayerMikeHandPressed", L"UIPlayerSabiHandPressed", L"UIPlayerKuroHandPressed" }
	};

    // Tutorial
    inline constexpr const wchar_t* TutorialBack = L"UITutorialBack";
    inline constexpr const wchar_t* TutorialStamp = L"UITutorialStamp";
    inline constexpr const wchar_t* TutorialToLeap = L"UITutorialToLeap";
    inline constexpr const wchar_t* TutorialSkip = L"UITutorialSkip";

    // Status
    inline constexpr const wchar_t* BossStatusBack = L"UIBossStatusBack";

    inline constexpr Players PlayerStatus{
        L"UIPlayerStatusMike",
        L"UIPlayerStatusSabi",
        L"UIPlayerStatusKuro"
    };

    inline constexpr const wchar_t* PlayerHeart = L"UIPlayerHeart";

    // Result
    inline constexpr const wchar_t* ClearResult = L"UIClearResult";
    inline constexpr const wchar_t* GameOverResult = L"UIGameOverResult";

    // Select
    inline constexpr Players SelectPlayers{
        L"UISelectMike",
        L"UISelectSabi",
        L"UISelectKuro"
    };
    inline constexpr Players SelectDKeys{
        L"UISelectMikeDKey",
        L"UISelectSabiDKey",
        L"UISelectKuroDKey"
    };
    inline constexpr Players SelectAKeys{
        L"UISelectMikeAKey",
        L"UISelectSabiAKey",
        L"UISelectKuroAKey"
	};

    inline constexpr const wchar_t* SelectCommon = L"UISelectCommon";

    // Player models
    inline constexpr Players Model_Player{
        L"ModelPlayerMike",
        L"ModelPlayerSabi",
        L"ModelPlayerKuro"
    };

    // Player animations
    inline constexpr const wchar_t* Model_PlayerAnim_Idle = L"PlayerAnimIdle";
    inline constexpr const wchar_t* Model_PlayerAnim_Walk = L"PlayerAnimWalk";
    inline constexpr const wchar_t* Model_PlayerAnim_Jump = L"PlayerAnimJump";
    inline constexpr const wchar_t* Model_PlayerAnim_Dodge = L"PlayerAnimDodge";
    inline constexpr const wchar_t* Model_PlayerAnim_Attack = L"PlayerAnimAttack";
    inline constexpr const wchar_t* Model_PlayerAnim_Ult = L"PlayerAnimUlt";
    inline constexpr const wchar_t* Model_PlayerAnim_SelectMike = L"PlayerAnimSelectMike";
    inline constexpr const wchar_t* Model_PlayerAnim_SelectSabi = L"PlayerAnimSelectSabi";

    // Boss
    inline constexpr const wchar_t* Model_Boss = L"ModelBoss";
    inline constexpr const wchar_t* Model_BossAnim_Idle = L"BossAnimIdle";
    inline constexpr const wchar_t* Model_BossAnim_Sippo = L"BossAnimSippo";
    inline constexpr const wchar_t* Model_BossAnim_Attck_Hit = L"BossAnimAtkHit";
    inline constexpr const wchar_t* Model_BossAnim_Down = L"BossAnimDown";
    inline constexpr const wchar_t* Model_BossAnim_Flinch = L"BossAnimFlinch";
    inline constexpr const wchar_t* Model_BossAnim_Die = L"BossAnimDie";
    inline constexpr const wchar_t* Model_BossAnim_Nekodamashi = L"BossAnimNekodamashi";

    // Stage models
    inline constexpr const wchar_t* Model_PillarL = L"hashira_l";
    inline constexpr const wchar_t* Model_PillarM = L"hashira_m";
    inline constexpr const wchar_t* Model_PillarS = L"hashira_s";
    inline constexpr const wchar_t* Model_Stage = L"Stage";
    inline constexpr const wchar_t* Model_stand_l = L"stand_l";
    inline constexpr const wchar_t* Model_stand_m = L"stand_m";
    inline constexpr const wchar_t* Model_stand_s = L"stand_s";
    inline constexpr const wchar_t* Model_JumpStand = L"JumpStand";

    // Room models
    inline constexpr const wchar_t* Model_Room = L"ModelRoom";
    inline constexpr const wchar_t* Model_Basket = L"ModelBasket";
    inline constexpr const wchar_t* Model_Chair = L"ModelChair";

    // SpriteStudio
    inline constexpr const wchar_t* SpriteStudio_Gauge = L"Gauge";
    inline constexpr const wchar_t* SpriteStudio_PlayerUltGauge = L"PlayerUltGauge";
    inline constexpr const wchar_t* SpriteStudio_BossHealthGauge = L"BossHealthGauge";
    inline constexpr const wchar_t* SpriteStudio_BossDownGauge = L"BossDownGauge";

    // Effects
    inline constexpr const wchar_t* Effect_Test = L"EffectTest";
    inline constexpr const wchar_t* Effect_Impact = L"EffectImpact";
    inline constexpr const wchar_t* Effect_PlayerAttack = L"EffectAttack";
    inline constexpr const wchar_t* Effect_Kedama = L"EffectKedama";

    // Music
    inline constexpr const wchar_t* Music_Title = L"MusicTitle";
    inline constexpr const wchar_t* Music_Battle = L"MusicBattle";

    // Sounds
    inline constexpr const wchar_t* Sound_MenuConfirm = L"SoundMenuConfirm";
    inline constexpr const wchar_t* Sound_BossEntrance = L"SoundBossEntrance";
	inline constexpr const wchar_t* Sound_hanko = L"SoundHanko";
	inline constexpr const wchar_t* Sound_Blink = L"SoundBlink";
	inline constexpr const wchar_t* Sound_Fall = L"SoundFall";

    inline constexpr Players SoundSelectPlayers{
        L"SoundSelectTaka",
        L"SoundSelectGishi",
        L"SoundSelectYu"
    };

    inline constexpr const wchar_t* Sound_Clap = L"SoundClap";
    inline constexpr const wchar_t* Sound_Slam = L"SoundSlam";
    inline constexpr const wchar_t* Sound_TailWhip = L"SoundTailWhip";

    // Fonts
    inline constexpr const wchar_t* Font_Title = L"Bitcount Light";
    inline constexpr const wchar_t* Font_Game = L"WDXL Lubrifont JP N";
}