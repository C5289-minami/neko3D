
#include "CameraDirector.h"

#include <algorithm>
#include <cmath>

namespace Camera
{
    namespace
    {
        constexpr float TwoPi =
            6.2831853071795864769f;
    }

    // =========================================================
    // Constructor
    // =========================================================

    CameraDirector::CameraDirector(Camera& camera)
        : activeCamera_(&camera)
        , currentPose_(camera.GetPose())
    {
    }

    // =========================================================
    // Shot Registration
    // =========================================================

    bool CameraDirector::RegisterShot(
        const std::string& name,
        const CameraPose& pose)
    {
        if (name.empty() || !IsFinitePose(pose))
        {
            return false;
        }

        shots_[name] = pose;
        return true;
    }

    bool CameraDirector::PlayShot(
        const std::string& name,
        float duration,
        Motion::Easing easing)
    {
        const auto it = shots_.find(name);

        if (it == shots_.end())
        {
            return false;
        }

        MoveTo(it->second, duration, easing);
        return true;
    }

    // =========================================================
    // MoveTo
    // =========================================================

    void CameraDirector::MoveTo(
        const CameraPose& targetPose,
        float duration,
        Motion::Easing easing)
    {
        if (!activeCamera_ || !IsFinitePose(targetPose))
        {
            return;
        }

        // 通常状態なら、カメラの最新姿勢を開始点にする
        if (mode_ == Mode::FollowingCamera)
        {
            currentPose_ = GetBasePose();
        }

        // 復帰用のMotionを停止
        followMotion_.Reset();

        // 以前の遷移を上書き
        transition_.Start(
            currentPose_,
            targetPose,
            duration,
            easing);

        mode_ = Mode::MovingToPose;
    }

    // =========================================================
    // SwitchCamera
    // =========================================================

    void CameraDirector::SwitchCamera(
        Camera& targetCamera,
        float duration,
        Motion::Easing easing)
    {
        if (&targetCamera == activeCamera_)
        {
            Follow(duration, easing);
            return;
        }

        // 切り替え前の姿勢を保存
        if (mode_ == Mode::FollowingCamera)
        {
            currentPose_ = GetBasePose();
        }

        // 切り替え前のカメラを保存
        previousCamera_ = activeCamera_;

        // 使用するカメラを切り替える
        activeCamera_ = &targetCamera;

        // 新しいカメラの姿勢へ補間
        BeginBlendToActiveCamera(duration, easing);
    }

    // =========================================================
    // Follow
    // =========================================================

    void CameraDirector::Follow(
        float duration,
        Motion::Easing easing)
    {
        if (!activeCamera_)
        {
            return;
        }

        if (mode_ == Mode::FollowingCamera)
        {
            currentPose_ = GetBasePose();
        }

        BeginBlendToActiveCamera(duration, easing);
    }

    // =========================================================
    // ReturnToPreviousCamera
    // =========================================================

    void CameraDirector::ReturnToPreviousCamera(
        float duration,
        Motion::Easing easing)
    {
        // 演出中なら、現在選択中のカメラ本来の姿勢に戻る
        if (mode_ == Mode::MovingToPose ||
            mode_ == Mode::HoldingPose)
        {
            Follow(duration, easing);
            return;
        }

        // 直前に使用していたカメラがあれば戻る
        if (previousCamera_ &&
            previousCamera_ != activeCamera_)
        {
            SwitchCamera(
                *previousCamera_,
                duration,
                easing);

            return;
        }

        Follow(duration, easing);
    }

    void CameraDirector::SetPoseImmediate(const CameraPose& pose)
    {
		// 遷移中のカメラがない場合は無視
        if (!activeCamera_ || !IsFinitePose(pose))
        {
            return;
        }

        // 実行中の遷移を停止
        transition_.Reset();
        followMotion_.Reset();

        // カメラの現在姿勢を直接更新
        currentPose_ = pose;
        mode_ = Mode::HoldingPose;

        activeCamera_->ApplyPose(currentPose_);
    }

    // =========================================================
    // BeginBlendToActiveCamera
    // =========================================================

    void CameraDirector::BeginBlendToActiveCamera(
        float duration,
        Motion::Easing easing)
    {
        if (!activeCamera_)
        {
            return;
        }

        transition_.Reset();

        // 即時切り替え
        if (duration <= 0.0f)
        {
            followMotion_.Reset();

            currentPose_ = GetBasePose();
            mode_ = Mode::FollowingCamera;

            activeCamera_->ApplyPose(currentPose_);
            return;
        }

        // 遷移開始時の姿勢を保存
        followStartPose_ = currentPose_;

        followMotion_.Start(
            0.0f,
            1.0f,
            duration,
            easing);

        mode_ = Mode::BlendingToCamera;
    }

    // =========================================================
    // Update
    // =========================================================

    bool CameraDirector::Update(float deltaTime)
    {
        if (!activeCamera_)
        {
            return false;
        }

        switch (mode_)
        {
        case Mode::FollowingCamera:
        {
            currentPose_ = GetBasePose();

            activeCamera_->ApplyPose(currentPose_);
            return false;
        }

        case Mode::MovingToPose:
        {
            const bool finished =
                transition_.Update(deltaTime, currentPose_);

            activeCamera_->ApplyPose(currentPose_);

            if (finished)
            {
                mode_ = Mode::HoldingPose;
            }

            return finished;
        }

        case Mode::HoldingPose:
        {
            // 演出完了後も姿勢を維持
            activeCamera_->ApplyPose(currentPose_);
            return false;
        }

        case Mode::BlendingToCamera:
        {
            // 最新のカメラ本来の姿勢を取得
            const CameraPose targetPose = GetBasePose();

            followMotion_.Update(deltaTime);

            const float t = followMotion_.GetValue();

            currentPose_ = InterpolatePose(
                followStartPose_,
                targetPose,
                t);

            activeCamera_->ApplyPose(currentPose_);

            if (followMotion_.IsFinished())
            {
                mode_ = Mode::FollowingCamera;
                return true;
            }

            return false;
        }
        }

        return false;
    }

    // =========================================================
    // Reset
    // =========================================================

    void CameraDirector::Reset()
    {
        transition_.Reset();
        followMotion_.Reset();

        previousCamera_ = nullptr;

        if (!activeCamera_)
        {
            return;
        }

        currentPose_ = GetBasePose();
        mode_ = Mode::FollowingCamera;

        activeCamera_->ApplyPose(currentPose_);
    }

    // =========================================================
    // Getters
    // =========================================================

    const CameraPose&
        CameraDirector::GetCurrentPose() const
    {
        return currentPose_;
    }

    Camera* CameraDirector::GetActiveCamera() const
    {
        return activeCamera_;
    }

    bool CameraDirector::IsTransitioning() const
    {
        return
            mode_ == Mode::MovingToPose ||
            mode_ == Mode::BlendingToCamera;
    }

    bool CameraDirector::IsFollowing() const
    {
        return mode_ == Mode::FollowingCamera;
    }

    // =========================================================
    // GetBasePose
    // =========================================================

    CameraPose CameraDirector::GetBasePose() const
    {
        if (!activeCamera_)
        {
            return {};
        }

        return activeCamera_->GetPose();
    }

    // =========================================================
    // Interpolation
    // =========================================================

    CameraPose CameraDirector::InterpolatePose(
        const CameraPose& from,
        const CameraPose& to,
        float t)
    {
        t = std::clamp(t, 0.0f, 1.0f);

        CameraPose pose{};

        // 位置
        pose.position =
            from.position +
            (to.position - from.position) * t;

        // 上下角度
        pose.pitch =
            from.pitch +
            (to.pitch - from.pitch) * t;

        // 左右角度（最短経路）
        pose.yaw = LerpAngle(
            from.yaw,
            to.yaw,
            t);

        // 傾き（最短経路）
        pose.roll = LerpAngle(
            from.roll,
            to.roll,
            t);

        // 画角
        pose.fieldOfViewDegrees =
            from.fieldOfViewDegrees +
            (to.fieldOfViewDegrees -
                from.fieldOfViewDegrees) * t;

        return pose;
    }

    float CameraDirector::LerpAngle(
        float from,
        float to,
        float t)
    {
        const float difference =
            std::remainder(to - from, TwoPi);

        return from + difference * t;
    }

    // =========================================================
    // Validation
    // =========================================================

    bool CameraDirector::IsFinitePose(
        const CameraPose& pose)
    {
        return
            std::isfinite(pose.position.x) &&
            std::isfinite(pose.position.y) &&
            std::isfinite(pose.position.z) &&
            std::isfinite(pose.pitch) &&
            std::isfinite(pose.yaw) &&
            std::isfinite(pose.roll) &&
            std::isfinite(pose.fieldOfViewDegrees);
    }
}
