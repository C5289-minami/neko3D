
#include "CameraTransition.h"
#include "DxLib.h"
#include <cmath>

namespace Camera
{
    namespace
    {
        constexpr float TwoPi = DX_PI_F * 2.0f;
    }

    void CameraTransition::Start(
        const CameraPose& startPose,
        const CameraPose& targetPose,
        float duration,
        Motion::Easing easing)
    {
        startPose_ = startPose;
        targetPose_ = targetPose;

        motion_.Start(0.0f, 1.0f, duration, easing);

        started_ = true;
        completionReported_ = false;
    }

    bool CameraTransition::Update(float deltaTime, CameraPose& outPose)
    {
        if (!started_)
        {
            return false;
        }

        motion_.Update(deltaTime);

        if (motion_.IsFinished())
        {
            // 完了時は目標姿勢をそのまま設定する
            outPose = targetPose_;

            // 完了した最初のフレームだけ true を返す
            if (!completionReported_)
            {
                completionReported_ = true;
                return true;
            }

            return false;
        }

        outPose = InterpolatePose(motion_.GetValue());
        return false;
    }

    bool CameraTransition::IsFinished() const
    {
        return started_ && motion_.IsFinished();
    }

    void CameraTransition::Reset()
    {
        motion_.Reset();

        startPose_ = {};
        targetPose_ = {};

        started_ = false;
        completionReported_ = false;
    }

    CameraPose CameraTransition::InterpolatePose(float t) const
    {
        CameraPose pose{};

        // 位置
        pose.position = startPose_.position
            + (targetPose_.position - startPose_.position) * t;

        // 上下角度
        pose.pitch = startPose_.pitch
            + (targetPose_.pitch - startPose_.pitch) * t;

        // 左右角度（最短経路）
        pose.yaw = LerpAngle(
            startPose_.yaw,
            targetPose_.yaw,
            t);

        // 傾き（最短経路）
        pose.roll = LerpAngle(
            startPose_.roll,
            targetPose_.roll,
            t);

        // 画角
        pose.fieldOfViewDegrees = startPose_.fieldOfViewDegrees
            + (targetPose_.fieldOfViewDegrees
                - startPose_.fieldOfViewDegrees) * t;

        return pose;
    }

    float CameraTransition::LerpAngle(float from, float to, float t)
    {
		// remainder を使って、角度の差を -π から π の範囲に収める
        const float difference = std::remainder(to - from, TwoPi);
        return from + difference * t;
    }
}
