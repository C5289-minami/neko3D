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
#include "PlayerUltimateEffect.h"
#include "Room.h"

class GameContext
{
public:
    GameContext() = default;					// コンストラクタ
    ~GameContext() = default;					// デストラクタ

    // ---- accessors ----

    // Colors
    int GetBgRed() const { return bgRed; }		// 背景色Rを取得
    void SetBgRed(int r) { bgRed = r; }			// 背景色Rを設定
    int GetBgGreen() const { return bgGreen; }	// 背景色Gを取得
    void SetBgGreen(int g) { bgGreen = g; }		// 背景色Gを設定
    int GetBgBlue() const { return bgBlue; }		// 背景色Bを取得
    void SetBgBlue(int b) { bgBlue = b; }		// 背景色Bを設定


    // Grid
    Grid& GetGrid() { return grid; }
    const Grid& GetGrid() const { return grid; }

	Boss& GetBoss() { return boss; }
	Player* GetPlayer() { return activePlayer; }//Playerのポインタを返す。nullptrの可能性あり
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
    void UpdateUltimateProjectiles(float deltaTime);
    void ClearUltimateProjectiles();
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

    PlayerUltimateEffect playerUltimateEffect_;

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