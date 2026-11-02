#pragma once

#include "UIManager.h"
#include <string>

class StatusUI
{
public:
    static void InitPlayer();
    static void InitBoss();

    static void SyncStatusBar(const std::wstring& key, float progress);
};