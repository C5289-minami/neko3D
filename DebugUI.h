// =============================
// DebugUI/DebugUI.h
// =============================
#pragma once
#include "GameContext.h"
#include "TransformSettings.h"

class DebugUI
{
public:
    void Init();
    void Shutdown();

    void BeginFrame();
    void Draw(GameContext& ctx, const DebugSceneControls& controls);
    void EndFrame();

private:
    void DrawTransformEditor(const DebugSceneControls& controls);
    std::string transformStatus_;
    std::string transformScope_;
    std::string selectedTargetId_;
    bool cameraSelected_{};
};
