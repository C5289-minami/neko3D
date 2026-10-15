// =============================
// Core/GameContext.h
// =============================
#pragma once
#include <string>

#include "Vector3.h"
#include "Grid.h"
#include "Stage.h"
#include "Player.h"
#include "Boss.h" 
#include "EffectManager.h"

class GameContext
{
public:
    GameContext() = default;					// ƒRƒ“ƒXƒgƒ‰ƒNƒ^
    ~GameContext() = default;					// ƒfƒXƒgƒ‰ƒNƒ^

    // ---- accessors ----

    // Colors
    int GetBgRed() const { return bgRed; }		// ”wŒiFR‚ğæ“¾
    void SetBgRed(int r) { bgRed = r; }			// ”wŒiFR‚ğİ’è
    int GetBgGreen() const { return bgGreen; }	// ”wŒiFG‚ğæ“¾
    void SetBgGreen(int g) { bgGreen = g; }		// ”wŒiFG‚ğİ’è
    int GetBgBlue() const { return bgBlue; }		// ”wŒiFB‚ğæ“¾
    void SetBgBlue(int b) { bgBlue = b; }		// ”wŒiFB‚ğİ’è

    void DrawStageDebugUI() { stage.DrawDebugUI(); }

    // Grid
    Grid& GetGrid() { return grid; }
    const Grid& GetGrid() const { return grid; }

    const Vec3& GetPlayerPosition() const { return player.GetPosition(); }
    void SetPlayerPosition(const Vec3& position) { player.SetPosition(position); }
    float GetPlayerUltimateGauge() const { return player.GetUltimateGauge(); }
    void SetPlayerUltimateGauge(float value) { player.SetUltimateGauge(value); }
    //d—Í
    bool IsPlayerGravityEnabled() const { return player.IsGravityEnabled(); }
    void SetPlayerGravityEnabled(bool enabled) { player.SetGravityEnabled(enabled); }
    //DebugSceneControls GetDebugControls();
	Boss& GetBoss() { return boss; }
	Player& GetPlayer() { return player; }
	Stage& GetStage() { return stage; }
	EffectManager& GetEffectManager() { return effectManager; }

    // ---- lifecycle ----
    void Init();
    void Reset();
    //void Draw() const;

private:
    // Colors
    int bgRed{ 24 };
    int bgGreen{ 160 };
    int bgBlue{ 224 };



    // Grid
    Grid grid;

	// Effect Manager
	EffectManager effectManager;

    // Stage
    Stage stage;
	// Player
	Player player;
    Boss boss;
    bool debugActorsPaused_{};
    std::string transformInitialStatus_;
};