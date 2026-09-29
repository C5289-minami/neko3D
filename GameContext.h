// =============================
// Core/GameContext.h
// =============================
#pragma once

#include "Vector3.h"
#include "Grid.h"
#include "OrbitCamera.h"
#include "Stage.h"
#include "Player.h" 

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

    // Orbit Camera
    OrbitCamera& GetOrbitCamera() { return orbitCamera; }
    const OrbitCamera& GetOrbitCamera() const { return orbitCamera; }

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
    OrbitCamera orbitCamera;
    bool wasOrbitControl{ false };

    Vec3 eye{ 400.0f, 400.0f, -400.0f };
    Vec3 target{};

    // TPS Camera
    float targetHeight{ 120.0f };
    float tpsDistance{ 250.0f };
    float tpsHeight{ 100.0f };

    // Stage
    Stage stage;
	// Player
	Player player;
};