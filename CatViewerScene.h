#pragma once

#include "Scene.h"
#include "Camera.h"
#include "ModelAnimation.h"

#include <string>
#include <vector>

// Owns a duplicate MV1 model. Neither gameplay transforms nor gameplay animation
// attachments are changed while inspecting a cat in this scene.
class CatViewerScene final : public Scene
{
public:
    explicit CatViewerScene(GameContext* context) : Scene(context) {}
    ~CatViewerScene() override { End(); }

    void Init() override;
    void Update(float deltaTime) override;
    void UpdatePresentationView(float realDeltaTime) override;
    void Render() const override;
    void DrawImGui() override;
    void End() override;

    const std::wstring& GetModelKey() const { return modelKey_; }
    bool SetModelKey(const std::wstring& modelKey);

private:
    void FitDisplayModel();
    void AttachClip(float time = 0.0f);
    float GetClipDuration() const;
    void ResetView();
    void DrawEffectImGui();
    void PlayPreviewEffect();
    void StopPreviewEffects();
    void SpawnPreviewEffects();
    void UpdatePreviewEffects(float deltaTime);
    void ApplyPreviewEffectSettings();
    void PrunePreviewEffects();

    // Only handles created by this viewer are owned/stopped here.
    std::vector<int> previewEffects_;
    std::string effectStatus_;
    int effectIndex_{};
    bool effectsPlaying_{};
    bool effectsPaused_{};
    bool repeatEffects_{};
    bool showCat_{ true };
    float effectScale_{ 1.0f };
    float effectSpeed_{ 1.0f };
    float effectPosition_[3]{ 0.0f, 2.0f, 0.0f };
    float effectYawDegrees_{};
    int dustCount_{ 4 };
    float dustRadius_{ 25.0f };
    float dustInterval_{ 0.18f };
    float dustRemaining_{};

    int modelHandle_{ -1 };
    std::wstring modelKey_;
    ModelAnimation animation_;
    Camera::Camera camera_;
    std::string status_;
    int clipIndex_{};
    bool playing_{ true };
    bool loop_{ true };
    bool showGrid_{ true };
    float playbackSpeed_{ 1.0f };
    float yaw_{ 35.0f };
    float pitch_{ 12.0f };
    float distance_{ 360.0f };
    float focusHeight_{ 60.0f };
};
