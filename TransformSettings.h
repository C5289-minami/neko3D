#pragma once

#include "Vector3.h"

#include <functional>
#include <optional>
#include <string>
#include <vector>

struct ObjectTransform
{
    Vec3 scale{ 1.0f, 1.0f, 1.0f };
    Vec3 position{};
    Vec3 rotation{}; // X・Y・Zの回転角（ラジアン）
};

struct DebugTransformTarget
{
    std::string id; // 保存用のID（シーン名で区別）
    std::string name;
    std::function<ObjectTransform()> get;
    std::function<void(const ObjectTransform&)> set;
};

template <typename Actor>
DebugTransformTarget MakeDebugTransformTarget(const char* id, const char* name, Actor& actor)
{
    return { id, name,
        [&actor] { return actor.GetTransform(); },
        [&actor](const ObjectTransform& transform) { actor.SetTransform(transform); } };
}

struct DebugCameraControls
{
    std::function<bool()> isActive;
    std::function<Vec3()> getPosition;
    std::function<void(const Vec3&)> setPosition;
    std::function<void()> reset;
    std::function<void()> focusPlayer;
};

struct DebugSceneControls
{
    std::string scope;
    std::vector<DebugTransformTarget> targets;
    std::optional<DebugCameraControls> camera;
    std::function<bool()> isPaused;
    std::function<void(bool)> setPaused;
    std::string initialStatus;
};

class TransformSettings
{
public:
    static const std::string& SourceFilePath();

    static bool IsValid(const ObjectTransform& transform);
    static bool Save(const std::vector<DebugTransformTarget>& targets, std::string& status,
        const std::string& sourcePath = SourceFilePath());
    static bool ResetToDefaults(const std::vector<DebugTransformTarget>& targets, std::string& status);
};
