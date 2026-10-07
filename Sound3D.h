#pragma once

#include "DxLib.h"
#include "Vector3.h"

class Sound3D
{
public:

    // 3D音源を再生
    static void Play(int soundHandle, Vec3 position,float radius = 1000.0f);

    // リスナー（プレイヤー）の位置・向きを設定
    static void SetListener(
        Vec3 position,
        Vec3 front
    );
};