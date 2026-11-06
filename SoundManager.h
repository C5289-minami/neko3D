#pragma once

#include "Consts.h"
#include "DxLib.h"
#include "Vector3.h"

class SoundManager
{
public:
    // BGM再生
    static void PlayBGM(int handle, int volume = Const::MAX_VOICE);

    // 通常のSE再生
    static void PlaySE(int handle, int volume = Const::MAX_VOICE);

    // 3D空間のSE再生
    static void PlaySE3D(int handle, Vec3 position, float radius = 1000.0f, int startTimeMs = 0);

    // BGM・SE停止
    static void StopBGM(int handle);
    static void StopSE(int handle);

    // リスナーの位置・向き設定
    static void SetListener(Vec3 position, Vec3 front);
};