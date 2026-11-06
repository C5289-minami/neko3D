#pragma once

/* - 仕様 -
 *  開始姿勢・目標姿勢・duration・イージングを指定できる。
 *  Motion の進行率に合わせて、位置・pitch・yaw・roll・画角を補間する。
 *  yaw と roll は最短角度で補間する。
 *  新しい Start() が呼ばれたら、進行中の遷移を上書きできる。
 *  Update() は遷移完了時に true を返す。完了済みかどうかは IsFinished() でも確認できる。
*/

#include "CameraPose.h"
#include "Motion.h"

namespace Camera
{
	// カメラ遷移を管理するクラス
    // このクラスはposeからposeまでの補完のみ管理
    class CameraTransition
    {
    public:
        // カメラ遷移を開始する
        void Start(
            const CameraPose& startPose,
            const CameraPose& targetPose,
            float duration,
            Motion::Easing easing = Motion::Easing::EaseInOut);

        // 遷移を更新し、完了したフレームで true を返す
        bool Update(float deltaTime, CameraPose& outPose);

        // 遷移が完了しているか
        bool IsFinished() const;

        // 遷移を中断する
        void Reset();

        // 現在の遷移進行率（0.0～1.0）
        float GetProgress() const { return motion_.GetValue(); }

    private:
        // 姿勢を補間する
        CameraPose InterpolatePose(float t) const;

        // 角度を最短経路で補間する
        static float LerpAngle(float from, float to, float t);

        CameraPose startPose_{};
        CameraPose targetPose_{};

        Motion motion_{};

        bool started_{};
        bool completionReported_{};
    };
}
