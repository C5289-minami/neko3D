// =============================
// Core/GameContext.h
// =============================
#pragma once
#include "Debug_camera.h"

#include "Vector3.h"
#include "Grid.h"
#include "15_System/Player_camera.h"
#include "Stage.h"
#include "Player.h"
#include "Boss.h" 

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

    // Player Camera
    Player_camera& GetPlayerCamera() { return playerCamera; }
    const Player_camera& GetPlayerCamera() const { return playerCamera; }
    const Vec3& GetPlayerPosition() const { return player.GetPosition(); }
    void SetPlayerPosition(const Vec3& position) { player.SetPosition(position); }
    const Vec3& GetCameraPosition() const;
    const Vec3& GetCameraTarget() const;
    const Vec3& GetCameraUp() const;
    void SetCameraPosition(const Vec3& position);
    bool IsSceneViewActive() const;
    float GetSceneCameraMoveSpeed() const { return Debug_camera.GetMoveSpeed(); }
    void SetSceneCameraMoveSpeed(float speed) { Debug_camera.SetMoveSpeed(speed); }
    void ResetSceneCamera();
    void FocusSceneCameraOnPlayer();

    // ---- lifecycle ----
    void Init();
    void Reset();
    void Update(float deltaTime);
    void Draw() const;

private:
    // Colors
    int bgRed{ 24 };
    int bgGreen{ 160 };
    int bgBlue{ 224 };

    // Grid
    Grid grid;

    // Camera
    Player_camera playerCamera;
    Debug_camera Debug_camera;

    // Stage
    Stage stage;
	// Player
	Player player;
    Boss boss;
};