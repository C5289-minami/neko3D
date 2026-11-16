#pragma once

#include "CameraDirector.h"

// A presentation view changes drawing only. Scripted cameras and gameplay keep
// updating underneath it, so boss events cannot replace the selected view.
class PresentationCamera
{
public:
    enum class Mode { Normal, InitialFollow, Free, CatFocus };

    void Reset(Camera::CameraDirector& director)
    {
        mode_ = Mode::Normal;
        needsInitialize_ = true;
        wasToggleDown_ = false;
        orbitYaw_ = 45.0f;
        orbitPitch_ = 12.0f;
        orbitDistance_ = 350.0f;
        focusHeight_ = 40.0f;
        director.ClearRenderOverride();
        freeCamera_.Reset();
    }

    Mode GetMode() const { return mode_; }

    void SetMode(Mode mode, Camera::CameraDirector& director)
    {
        if (mode == mode_) return;
        freeCamera_.Initialize(director.GetRenderPose());
        homePose_ = director.GetRenderPose();
        mode_ = mode;
        needsInitialize_ = true;
        if (mode_ == Mode::Normal) director.ClearRenderOverride();
    }

    void Update(float realDeltaTime, const Vec3& playerPosition,
        const Vec3& bossPosition, Camera::CameraDirector& director)
    {
#ifndef NDEBUG
        const bool keyboardCaptured = ImGui::GetCurrentContext() &&
            ImGui::GetIO().WantTextInput;
        const bool mouseCaptured = ImGui::GetCurrentContext() &&
            ImGui::GetIO().WantCaptureMouse;
        const bool toggleDown = DxLib::CheckHitKey(KEY_INPUT_LALT) &&
            DxLib::CheckHitKey(KEY_INPUT_RETURN);
        if (toggleDown && !wasToggleDown_ && !keyboardCaptured)
            SetMode(mode_ == Mode::Free ? Mode::Normal : Mode::Free, director);
        wasToggleDown_ = toggleDown;

        if (mode_ == Mode::Normal)
        {
            director.ClearRenderOverride();
            return;
        }

        realDeltaTime = std::isfinite(realDeltaTime)
            ? std::clamp(realDeltaTime, 0.0f, 0.1f) : 0.0f;
        if (mode_ == Mode::InitialFollow)
        {
            // Recreate the early horizontal boss-to-player tracking formula.
            Vec3 away = playerPosition - bossPosition;
            away.y = 0.0f;
            away = away.LengthSq() > Const::EPS ? away.Normalized() : Vec3{ 0, 0, 1 };
            const Vec3 desiredEye = playerPosition + away * 900.0f + Vec3{ 0, 420, 0 };
            const Vec3 desiredTarget = (playerPosition + bossPosition) * 0.5f + Vec3{ 0, 110, 0 };
            if (needsInitialize_)
            {
                followEye_ = desiredEye;
                followTarget_ = desiredTarget;
            }
            const float amount = std::clamp(5.0f * realDeltaTime, 0.0f, 1.0f);
            followEye_ += (desiredEye - followEye_) * amount;
            followTarget_ += (desiredTarget - followTarget_) * amount;
            viewCamera_.Initialize(followEye_, followTarget_);
            pose_ = viewCamera_.GetPose();
            pose_.fieldOfViewDegrees = 60.0f;
        }
        else if (mode_ == Mode::Free)
        {
            // The pause chord must not also fly the camera up/down. FreeCamera
            // itself handles ImGui capture; it is updated only on the real clock.
            const bool pauseChord = DxLib::CheckHitKey(KEY_INPUT_Q) && DxLib::CheckHitKey(KEY_INPUT_E);
            if (!mouseCaptured && !pauseChord &&
                (DxLib::GetMouseInput() & MOUSE_INPUT_RIGHT) != 0)
                freeCamera_.Update(realDeltaTime);
            pose_ = freeCamera_.GetPose();
        }
        else
        {
            if (!mouseCaptured && !keyboardCaptured &&
                (DxLib::GetMouseInput() & MOUSE_INPUT_RIGHT) != 0)
            {
                const auto mouse = DxPlus::Input::GetMouseDelta();
                orbitYaw_ = std::remainder(orbitYaw_ + mouse.x * 0.2f, 360.0f);
                orbitPitch_ = std::clamp(orbitPitch_ + mouse.y * 0.2f, -75.0f, 75.0f);
            }
            const Vec3 target = playerPosition + Vec3{ 0, focusHeight_, 0 };
            const float yaw = orbitYaw_ * DxPlus::Deg2Rad;
            const float pitch = orbitPitch_ * DxPlus::Deg2Rad;
            const Vec3 offset{
                std::sin(yaw) * std::cos(pitch), std::sin(pitch),
                std::cos(yaw) * std::cos(pitch)
            };
            viewCamera_.Initialize(target + offset * orbitDistance_, target);
            pose_ = viewCamera_.GetPose();
            pose_.fieldOfViewDegrees = 45.0f;
        }
        needsInitialize_ = false;
        director.SetRenderOverride(pose_);
#else
        (void)realDeltaTime;
        (void)playerPosition;
        (void)bossPosition;
        director.ClearRenderOverride();
#endif
    }

    void DrawImGui(const Vec3& playerPosition, Camera::CameraDirector& director)
    {
#ifndef NDEBUG
        ImGui::Separator();
        if (!ImGui::CollapsingHeader(u8"発表カメラ", ImGuiTreeNodeFlags_DefaultOpen)) return;
        const char* const names[] = {
            u8"通常カメラ", u8"初期風の追従カメラ（再現）", u8"自由カメラ", u8"猫に注目するカメラ"
        };
        int selection = static_cast<int>(mode_);
        if (ImGui::Combo(u8"種類", &selection, names, 4))
            SetMode(static_cast<Mode>(selection), director);
        ImGui::TextWrapped(u8"通常以外は発表用の視点を維持します。ゲームの進行・演出は継続します。");
        ImGui::TextUnformatted(u8"Alt + Enter: 自由カメラ / 通常カメラ");
        if (mode_ == Mode::InitialFollow)
            ImGui::TextWrapped(u8"初期の式を再現：水平追従、距離900 / 高さ420 / 注視点110 / 追従速度5。");
        if (mode_ == Mode::Free)
        {
            ImGui::TextWrapped(u8"右ドラッグで回転、右ボタン + WASDで移動、Q/Eで下降/上昇、Shiftで加速。Q+Eは時間停止。");
            if (ImGui::Button(u8"自由カメラを開始位置へ戻す")) freeCamera_.Initialize(homePose_);
            if (ImGui::Button(u8"猫の現在位置に注目")) freeCamera_.FocusAt(playerPosition + Vec3{ 0, 40, 0 });
        }
        if (mode_ == Mode::CatFocus)
        {
            ImGui::TextUnformatted(u8"右ドラッグで猫の周囲を回転");
            ImGui::SliderFloat(u8"距離", &orbitDistance_, 80.0f, 1200.0f, "%.0f");
            ImGui::SliderFloat(u8"上下角度", &orbitPitch_, -75.0f, 75.0f, "%.0f");
            ImGui::SliderFloat(u8"注目する高さ", &focusHeight_, 0.0f, 200.0f, "%.0f");
            if (ImGui::Button(u8"正面")) orbitYaw_ = 0.0f;
            ImGui::SameLine();
            if (ImGui::Button(u8"横")) orbitYaw_ = 90.0f;
            ImGui::SameLine();
            if (ImGui::Button(u8"後ろ")) orbitYaw_ = 180.0f;
        }
#else
        (void)playerPosition;
        (void)director;
#endif
    }

private:
    Mode mode_{ Mode::Normal };
    bool needsInitialize_{ true };
    bool wasToggleDown_{};
    Camera::FreeCamera freeCamera_;
    Camera::Camera viewCamera_;
    Camera::CameraPose pose_{};
    Camera::CameraPose homePose_{};
    Vec3 followEye_{};
    Vec3 followTarget_{};
    float orbitYaw_{ 45.0f };
    float orbitPitch_{ 12.0f };
    float orbitDistance_{ 350.0f };
    float focusHeight_{ 40.0f };
};
