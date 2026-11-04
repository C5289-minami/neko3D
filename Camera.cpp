
#include "Camera.h"
#include "Consts.h"

#include <algorithm>
#include <cmath>

namespace Camera
{
    // =========================================================
    // Camera
    // =========================================================

    CameraPose Camera::GetPose() const
    {
        CameraPose pose{};

        pose.position = position_;
        pose.pitch = pitch_;
        pose.yaw = yaw_;
        pose.roll = roll_;
        pose.fieldOfViewDegrees = GetFieldOfView();

        return pose;
    }

    void Camera::ApplyPose(const CameraPose& pose) const
    {
        // 不正な値を適用しない
        if (!std::isfinite(pose.position.x) ||
            !std::isfinite(pose.position.y) ||
            !std::isfinite(pose.position.z) ||
            !std::isfinite(pose.pitch) ||
            !std::isfinite(pose.yaw) ||
            !std::isfinite(pose.roll) ||
            !std::isfinite(pose.fieldOfViewDegrees))
        {
            return;
        }

        constexpr float TwoPi = 6.2831853071795864769f;

        const float pitch = std::clamp(
            pose.pitch,
            -89.0f * DxPlus::Deg2Rad,
            89.0f * DxPlus::Deg2Rad);

        const float yaw = std::remainder(pose.yaw, TwoPi);
        const float roll = std::remainder(pose.roll, TwoPi);

        const float fieldOfView = std::clamp(
            pose.fieldOfViewDegrees,
            10.0f,
            150.0f);

        // カメラの前方向
        const float cosPitch = std::cos(pitch);

        const Vec3 forward{
            std::sin(yaw) * cosPitch,
            std::sin(pitch),
            std::cos(yaw) * cosPitch
        };

        // カメラの右方向
        Vec3 right{
            forward.z,
            0.0f,
            -forward.x
        };

        if (right.LengthSq() < Const::EPS)
        {
            right = { 1.0f, 0.0f, 0.0f };
        }
        else
        {
            right = right.Normalized();
        }

        // カメラの上方向
        const Vec3 up{
            forward.y * right.z,
            forward.z * right.x - forward.x * right.z,
            -forward.y * right.x
        };

        // ロールを反映
        const Vec3 rolledUp =
            up * std::cos(roll) + right * std::sin(roll);

        // 画角を設定
        DxLib::SetupCamera_Perspective(
            fieldOfView * DxPlus::Deg2Rad);

        // Near / Farを設定
        DxLib::SetCameraNearFar(
            nearClipDistance_,
            farClipDistance_);

        // カメラの位置・向き・上方向を設定
        DxLib::SetCameraPositionAndTargetAndUpVec(
            DxConv::ToVECTOR(pose.position),
            DxConv::ToVECTOR(pose.position + forward),
            DxConv::ToVECTOR(rolledUp));
    }

    void Camera::ApplyCamera() const
    {
        ApplyPose(GetPose());
    }

    // =========================================================
    // TargetCamera
    // =========================================================

    CameraPose TargetCamera::GetPose() const
    {
        CameraPose pose{};

        pose.position = position_;

        // 追従カメラが計算した実際の視線角度を使用
        const Vec3 angles = GetViewAnglesDegrees();

        pose.pitch = angles.x * DxPlus::Deg2Rad;
        pose.yaw = angles.y * DxPlus::Deg2Rad;
        pose.roll = angles.z * DxPlus::Deg2Rad;

        pose.fieldOfViewDegrees = GetFieldOfView();

        return pose;
    }
    float Camera::GetNearClipDistance()
    {
        return nearClipDistance_;
    }

    float Camera::GetFarClipDistance()
    {
        return farClipDistance_;
    }

    bool Camera::SetNearFar(
        float nearDistance,
        float farDistance)
    {
        if (!std::isfinite(nearDistance) ||
            !std::isfinite(farDistance) ||
            nearDistance <= 0.0f ||
            farDistance <= nearDistance)
        {
            return false;
        }

        nearClipDistance_ = nearDistance;
        farClipDistance_ = farDistance;

        return true;
    }
    void Camera::ApplyView(const Vec3& target) const
    {
        Vec3 forward = (target - position_).Normalized();

        Vec3 right{
            forward.z,
            0.0f,
            -forward.x
        };

        if (right.LengthSq() < Const::EPS)
        {
            right = { 1.0f, 0.0f, 0.0f };
        }
        else
        {
            right = right.Normalized();
        }

        const Vec3 up{
            forward.y * right.z,
            forward.z * right.x - forward.x * right.z,
            -forward.y * right.x
        };

        const Vec3 rolledUp =
            up * std::cos(roll_) +
            right * std::sin(roll_);

        DxLib::SetupCamera_Perspective(
            fieldOfViewDegrees_ * DxPlus::Deg2Rad);

        DxLib::SetCameraPositionAndTargetAndUpVec(
            DxConv::ToVECTOR(position_),
            DxConv::ToVECTOR(target),
            DxConv::ToVECTOR(rolledUp));
    }
}
