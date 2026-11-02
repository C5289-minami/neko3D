#include "DrawableObject.h"
#include "ActorBase.h"
#include "DxConv.h"
#include "ResourceManager.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

// ============================================================
// ModelObject
// ============================================================

void ModelObject::Init()
{
    if (!Load())
    {
        position = {};
        scale = { 1.0f, 1.0f, 1.0f };
        rotation = {};
    }

    ApplyTransform();
}

bool ModelObject::Load(const std::string& filePath)
{
    if (modelKey.empty() || objectId.empty()) return false;

    try
    {
        std::ifstream file(filePath);
        if (!file) return false;

        nlohmann::json json;
        file >> json;

        const auto objects = json.find("objects");
        if (objects == json.end() || !objects->is_object()) return false;

        const auto saved = objects->find(objectId);
        if (saved == objects->end() || !saved->is_object()) return false;

        const auto readVector = [](const nlohmann::json& value, Vec3& output)
            {
                if (!value.is_array() || value.size() != 3) return false;

                output = {
                    value[0].get<float>(),
                    value[1].get<float>(),
                    value[2].get<float>()
                };

                return std::isfinite(output.x)
                    && std::isfinite(output.y)
                    && std::isfinite(output.z);
            };

        Vec3 loadedPosition = position;
        Vec3 loadedScale = scale;
        Vec3 loadedRotation = rotation;

        if (!readVector(saved->at("position"), loadedPosition)
            || !readVector(saved->at("scale"), loadedScale)
            || !readVector(saved->at("rotation"), loadedRotation)
            || loadedScale.x <= 0.0f
            || loadedScale.y <= 0.0f
            || loadedScale.z <= 0.0f)
        {
            return false;
        }

        position = loadedPosition;
        scale = loadedScale;
        rotation = loadedRotation;

        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

bool ModelObject::Save(const std::string& filePath) const
{
    if (modelKey.empty() || objectId.empty()
        || !std::isfinite(position.x)
        || !std::isfinite(position.y)
        || !std::isfinite(position.z)
        || !std::isfinite(scale.x)
        || !std::isfinite(scale.y)
        || !std::isfinite(scale.z)
        || !std::isfinite(rotation.x)
        || !std::isfinite(rotation.y)
        || !std::isfinite(rotation.z)
        || scale.x <= 0.0f
        || scale.y <= 0.0f
        || scale.z <= 0.0f)
    {
        return false;
    }

    try
    {
        nlohmann::json json = nlohmann::json::object();

        std::ifstream input(filePath);
        if (input)
        {
            input >> json;
            if (!json.is_object()) return false;
        }

        if (!json.contains("objects") || !json["objects"].is_object())
            json["objects"] = nlohmann::json::object();

        json["objects"][objectId] = {
            { "position", { position.x, position.y, position.z } },
            { "scale", { scale.x, scale.y, scale.z } },
            { "rotation", { rotation.x, rotation.y, rotation.z } }
        };

        const std::filesystem::path path(filePath);
        if (!path.parent_path().empty())
            std::filesystem::create_directories(path.parent_path());

        std::ofstream output(filePath, std::ios::trunc);
        if (!output) return false;

        output << json.dump(4);
        return output.good();
    }
    catch (const std::exception&)
    {
        return false;
    }
}

// ============================================================
// ImageObject
// ============================================================

DxPlus::Vec2 ImageObject::GetSize() const
{
    const auto* sprite = RM().GetSprite(imageKey);
    if (!sprite || !sprite->IsLoaded()) return {};

    DxPlus::Vec2 size{};
    DxLib::GetGraphSizeF(sprite->GetID(), &size.x, &size.y);
    return size;
}

void ImageObject::Draw(DxPlus::Vec2 masterPos) const
{
    const auto* sprite = RM().GetSprite(imageKey);
    if (!sprite) return;

    const DxPlus::Vec2 size = GetSize();
    const DxPlus::Vec2 pivot{ center.x * size.x, center.y * size.y };

    sprite->Draw(position + masterPos, scale, pivot, rotation);
}

// ============================================================
// ModelObject: Transform
// ============================================================

// ローカル行列と親のワールド行列を合成
Matrix4 ModelObject::GetWorldMatrix() const
{
    const Matrix4 local = GetTransformMatrix();
    return owner_ ? local * owner_->GetWorldMatrix() : local;
}

void ModelObject::ApplyTransform(Vec3 masterPos) const
{
    const int handle = RM().GetModel(modelKey);
    if (handle < 0) return;

    Matrix4 worldMatrix = GetWorldMatrix();

    // 追加の位置オフセット
    Vec3 worldPosition = worldMatrix.GetTranslation();
    worldPosition.x += masterPos.x;
    worldPosition.y += masterPos.y;
    worldPosition.z += masterPos.z;

    // 回転を考慮して中心を補正
    const Vec3 worldRotation = worldMatrix.GetRotationXYZ();
    const Vec3 rotatedCenter =
        Matrix4::CreateRotationXYZ(worldRotation).TransformVector(center);

    worldPosition.x -= rotatedCenter.x;
    worldPosition.y -= rotatedCenter.y;
    worldPosition.z -= rotatedCenter.z;

    worldMatrix.SetTranslation(worldPosition);

    // 行列をモデルに適用
    MV1SetMatrix(handle, DxConv::ToMATRIX(worldMatrix));
}

void ModelObject::Draw(Vec3 masterPos) const
{
    const int handle = RM().GetModel(modelKey);
    if (handle < 0) return;

    ApplyTransform(masterPos);
    ApplyCollisionColor();
    DxLib::MV1SetOpacityRate(handle, alpha);
    DxLib::MV1DrawModel(handle);
    DxLib::MV1SetOpacityRate(handle, 1.0f);
}

ModelObject ModelObject::WithParentTransform(
    const Vec3& parentPosition,
    const Vec3& parentRotation) const
{
    ModelObject world = *this;
    world.owner_ = nullptr;

    const Matrix4 parentMatrix =
        Matrix4::CreateRotationXYZ(parentRotation)
        * Matrix4::CreateTranslation(parentPosition);

    const Matrix4 worldMatrix = GetTransformMatrix() * parentMatrix;

    // 合成結果を位置・回転・スケールに戻す
    world.position = worldMatrix.GetTranslation();
    world.rotation = worldMatrix.GetRotationXYZ();
    world.scale = worldMatrix.GetScale();

    return world;
}

// ============================================================
// ModelObject: Other
// ============================================================

void ModelObject::FadeTo(
    const ActorBase& player,
    float speed,
    float deltaTime)
{
    if (player.GetPosition().y >= GetPosition().y)
    {
        SetAlpha(1.0f);
        return;
    }

    const float step = (std::max)(speed, 0.0f)
        * (std::max)(deltaTime, 0.0f);

    if (alpha <= step)
    {
        SetAlpha(0.0f);
        return;
    }

    SetAlpha(alpha - step);
}

Collision::Ellipsoid ModelObject::GetHitEllipsoid() const
{
    const int handle = RM().GetModel(modelKey);
    if (handle < 0) return {};

    ApplyTransform();

    if (MV1SetupReferenceMesh(handle, -1, TRUE, TRUE) < 0)
        return {};

    const int refreshResult =
        MV1RefreshReferenceMesh(handle, -1, TRUE, TRUE);

    const auto mesh = MV1GetReferenceMesh(handle, -1, TRUE, TRUE);

    const Vec3 minimum = DxConv::ToVec3(mesh.MinPosition);
    const Vec3 maximum = DxConv::ToVec3(mesh.MaxPosition);
    const bool valid = refreshResult >= 0 && mesh.VertexNum > 0;

    DxLib::MV1TerminateReferenceMesh(handle, -1, TRUE, TRUE);

    if (!valid) return {};

    return {
        (minimum + maximum) * 0.5f,
        (maximum - minimum) * 0.5f
    };
}

void ModelObject::ApplyCollisionColor() const
{
    const int handle = RM().GetModel(modelKey);
    if (handle < 0) return;

    const auto color = collisionHighlighted
        ? GetColorF(1.0f, 0.0f, 0.0f, 1.0f)
        : GetColorF(1.0f, 1.0f, 1.0f, 1.0f);

    MV1SetDifColorScale(handle, color);
    MV1SetAmbColorScale(handle, color);
    MV1SetSpcColorScale(handle, color);
    MV1SetEmiColorScale(handle, color);
}

// ============================================================
// Collision
// ============================================================

namespace Collision
{
    bool UpdateModelContact(ModelObject& a, ModelObject& b)
    {
        const bool touching =
            IsHitEllipsoidEllipsoid(a.GetHitEllipsoid(), b.GetHitEllipsoid());

        a.collisionHighlighted = touching;
        b.collisionHighlighted = touching;

        return touching;
    }
}