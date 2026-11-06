#pragma once

#include "UIManager.h"
#include <string>

class Player;

class StatusUI
{
public:
    static void InitPlayer(const Player* player = nullptr);
    static void InitBoss();

    static void Open();
    static void Hide();
    static void Remove();

    static void SyncStatusBar(const std::wstring& key, float progress);
};