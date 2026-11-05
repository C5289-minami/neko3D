// =============================
// Core/GameContext.h
// =============================
#pragma once
#include <string>
#include <array>
#include "Vector3.h"


#include "Grid.h"
#include "Stage.h"
#include "Player.h"
#include "Boss.h" 
#include "EffectManager.h"
#include "Room.h"

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


    // Grid
    Grid& GetGrid() { return grid; }
    const Grid& GetGrid() const { return grid; }

	Boss& GetBoss() { return boss; }
    Player* GetPlayer() { return activePlayer; }
    void SetActivePlayer(Player& player)
    {
        activePlayer = &player;
        boss.SetPlayer(player);
    }
	Stage& GetStage() { return stage; }
	EffectManager& GetEffectManager() { return effectManager; }
	Room& GetRoom() { return room_; }

	// Select Preview
    std::array<Player, 3>& GetSelectPreviewPlayers()
    {
        return selectPreviewPlayers_;
    }

    const std::array<Player, 3>& GetSelectPreviewPlayers() const
    {
        return selectPreviewPlayers_;
    }

    bool IsSelectPreviewVisible() const
    {
        return selectPreviewVisible_;
    }

    void SetSelectPreviewVisible(bool visible)
    {
        selectPreviewVisible_ = visible;
    }

    // ---- lifecycle ----
    void Init();
    void Reset();
    //void Draw() const;

private:
    // Colors
    int bgRed{ 235 };
    int bgGreen{ 235 };
    int bgBlue{ 235 };



    // Grid
    Grid grid;

	// Effect Manager
	EffectManager effectManager;

	// Actors
    Stage stage;
    Room room_;
    Player defaultPlayer;
    Player* activePlayer = &defaultPlayer;

    Boss boss;

    bool debugActorsPaused_{};
    std::string transformInitialStatus_;

    std::array<Player, 3> selectPreviewPlayers_;
    bool selectPreviewVisible_{ false };
};