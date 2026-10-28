#pragma once

#include <string>
#include "Consts.h"

struct OutlineSettings
{
    // 外側：モデルの輪郭
    float outerRadius = 2.0f;
    float depthStrength = 20.0f;
    float outerOpacity = 1.0f;
    float padding0 = 0.0f;

    // 内側：法線エッジ
    float innerRadius = 2.0f;
    float normalStrength = 5.0f;
    float normalThreshold = 0.1f;
    float innerOpacity = 1.0f;

    // 共通カラー
    float outlineColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    // カメラの深度変換用
    float cameraNear = Const::CAMERA_NEAR;
    float cameraFar = Const::CAMERA_FAR;
    float padding1 = 0.0f;
    float padding2 = 0.0f;
};

inline OutlineSettings g_outlineSettings;

class OutlineSettingsManager
{
public:
    static bool Save(
        const std::string& filePath = "./Data/Config/outline.json");

    static bool Load(
        const std::string& filePath = "./Data/Config/outline.json");

    static void Reset();
};