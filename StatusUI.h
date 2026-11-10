#pragma once

#include "UIManager.h"
#include <string>

class Player;

class StatusUI
{
public:
    static void InitPlayer(const std::string& characterName = "たか");
    static void InitBoss();

    static void Open(int playerHealth);
    static void Hide();
    static void Remove();

    static void SyncStatusBar(const std::wstring& key, float progress);
    static void SyncPlayerHealth(int health, float deltaTime);
    static bool IsPlayerHeartLossAnimating();
};
