// =============================
// DebugUI/DebugUI.h
// =============================
#pragma once
#include "../GameContext.h"

class DebugUI
{
public:
    void Init();
    void Shutdown();

    void BeginFrame();
    void Draw(GameContext& ctx);
    void EndFrame();
};
