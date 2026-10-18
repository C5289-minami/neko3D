
#pragma once

#include "Vector3.h"

namespace Camera
{
    // カメラの姿勢を表すデータ
    struct CameraPose
    {
        Vec3 position{};

        // 傾き
        float pitch{}; 
        float yaw{};   
        float roll{};   

        float fieldOfViewDegrees{ 60.0f }; // 画角（度）
    };
}

