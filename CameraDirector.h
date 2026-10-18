
#pragma once

#include "Camera.h"
#include "CameraTransition.h"
#include "Motion.h"

#include <string>
#include <unordered_map>

namespace Camera
{
    class CameraDirector
    {
    public:
        // 使用するカメラを指定して初期化
        explicit CameraDirector(Camera& camera);

        // =====================================================
        // 演出用アングルの登録・再生
        // =====================================================

        // 名前を付けて演出用アングルを登録
        // 同じ名前が存在する場合は上書き
        bool RegisterShot(
            const std::string& name,
            const CameraPose& pose);

        // 登録したアングルを削除
        bool UnregisterShot(const std::string& name);

        // 登録済みのアングルが存在するか
        bool HasShot(const std::string& name) const;

        // 登録したアングルを再生
        bool PlayShot(
            const std::string& name,
            float duration,
            Motion::Easing easing = Motion::Easing::EaseInOut);

        // =====================================================
        // 姿勢の制御
        // =====================================================

        // 指定した姿勢へ移動
        // 移動中の再呼び出しは割り込みとして扱う
        void MoveTo(
            const CameraPose& targetPose,
            float duration,
            Motion::Easing easing = Motion::Easing::EaseInOut);

        // 指定した姿勢へ即座に変更し、その姿勢を維持
        void SetPose(const CameraPose& pose);

        // 使用するカメラを切り替える
        // 例：FreeCamera から TargetCamera へ
        void SwitchCamera(
            Camera& targetCamera,
            float duration = 0.0f,
            Motion::Easing easing = Motion::Easing::EaseInOut);

        // 現在のカメラ本来の姿勢へ戻る
        void Follow(
            float duration = 0.0f,
            Motion::Easing easing = Motion::Easing::EaseInOut);

        // 演出中なら現在のカメラへ戻る。
        // 通常カメラの使用中なら、直前のカメラへ戻る。
        void ReturnToPreviousCamera(
            float duration = 0.0f,
            Motion::Easing easing = Motion::Easing::EaseInOut);

        // =====================================================
        // 更新・状態取得
        // =====================================================

        // 毎フレーム呼び出す。
        // 遷移が完了したフレームだけ true を返す。
        bool Update(float deltaTime);

        // 現在のカメラ状態へ戻して初期化
        void Reset();

        // 現在画面に適用する姿勢
        const CameraPose& GetCurrentPose() const;

        // 現在選択されているカメラ
        Camera* GetActiveCamera() const;

        // 遷移中か
        bool IsTransitioning() const;

        // カメラ本来の姿勢を使用中か
        bool IsFollowing() const;

        // 固定姿勢を維持中か
        bool IsHoldingPose() const;

    private:
        enum class Mode
        {
            FollowingCamera,     // カメラ本来の姿勢を使用
            MovingToPose,        // 指定姿勢へ移動中
            HoldingPose,         // 指定姿勢を維持
            BlendingToCamera     // カメラ本来の姿勢へ復帰中
        };

        // 現在のカメラへ滑らかに戻る処理を開始
        void BeginBlendToActiveCamera(
            float duration,
            Motion::Easing easing);

        // カメラ本来の姿勢を取得
        CameraPose GetBasePose() const;

        // 姿勢を補間
        static CameraPose InterpolatePose(
            const CameraPose& from,
            const CameraPose& to,
            float t);

        // 角度を最短経路で補間（ラジアン）
        static float LerpAngle(
            float from,
            float to,
            float t);

        // 数値が有効な姿勢か
        static bool IsFinitePose(const CameraPose& pose);

        // 現在使用しているカメラ
        Camera* activeCamera_{};

        // 切り替え前に使用していたカメラ
        Camera* previousCamera_{};

        // 登録済み演出アングル
        std::unordered_map<std::string, CameraPose> shots_{};

        // 固定姿勢への遷移
        CameraTransition transition_{};

        // カメラ本来の姿勢への復帰
        Motion followMotion_{};

        // 現在表示している姿勢
        CameraPose currentPose_{};

        // 復帰開始時の姿勢
        CameraPose followStartPose_{};

        Mode mode_{ Mode::FollowingCamera };
    };
}
