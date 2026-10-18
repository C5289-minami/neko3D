// =============================
// DebugUI/DebugUI.h
// =============================
#pragma once
#include "GameContext.h"


class DebugUI
{
public:
    void Init();
    void Shutdown();

    void BeginFrame();
    void Draw(GameContext& ctx);
    void EndFrame();

private:
    void DrawScreenSizeControls();
    int screenSize_[2]{};
    std::string screenSizeStatus_;
    std::string transformStatus_;
    std::string transformScope_;
    std::string selectedTargetId_;
    bool cameraSelected_{};
};
