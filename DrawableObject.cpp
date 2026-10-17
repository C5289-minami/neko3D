#include "DrawableObject.h"
#include "DxConv.h"
#include "ResourceManager.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include "ActorBase.h"

namespace
{
    using RotationMatrix = std::array<std::array<float, 3>, 3>;

	// ヘルパー関数: std::wstringをstd::stringに変換する
    std::string NarrowModelKey(const std::wstring& key)
    {
        return { key.begin(), key.end() };
    }

	// ヘルパー関数: 回転を考慮してベクトルを回転させる
    Vec3 RotateXYZ(const Vec3& value, const Vec3& rotation)
    {
        const float cosX = std::cos(rotation.x);
        const float sinX = std::sin(rotation.x);
        const float cosY = std::cos(rotation.y);
        const float sinY = std::sin(rotation.y);
        const float cosZ = std::cos(rotation.z);
        const float sinZ = std::sin(rotation.z);

        const Vec3 rotatedX{
            value.x,
            value.y * cosX - value.z * sinX,
            value.y * sinX + value.z * cosX
        };
        const Vec3 rotatedY{
            rotatedX.x * cosY + rotatedX.z * sinY,
            rotatedX.y,
            -rotatedX.x * sinY + rotatedX.z * cosY
        };

        return Vec3{
            rotatedY.x * cosZ - rotatedY.y * sinZ,
            rotatedY.x * sinZ + rotatedY.y * cosZ,
            rotatedY.z
        };
    }

    RotationMatrix MakeRotationMatrix(const Vec3& rotation)
    {
        const float cosX = std::cos(rotation.x);
        const float sinX = std::sin(rotation.x);
        const float cosY = std::cos(rotation.y);
        const float sinY = std::sin(rotation.y);
        const float cosZ = std::cos(rotation.z);
        const float sinZ = std::sin(rotation.z);

        return {{
            { cosY * cosZ, sinX * sinY * cosZ - cosX * sinZ, cosX * sinY * cosZ + sinX * sinZ },
            { cosY * sinZ, sinX * sinY * sinZ + cosX * cosZ, cosX * sinY * sinZ - sinX * cosZ },
            { -sinY, sinX * cosY, cosX * cosY }
        }};
    }

    RotationMatrix Multiply(const RotationMatrix& left, const RotationMatrix& right)
    {
        RotationMatrix result{};
        for (std::size_t row = 0; row < 3; ++row)
            for (std::size_t column = 0; column < 3; ++column)
                for (std::size_t index = 0; index < 3; ++index)
                    result[row][column] += left[row][index] * right[index][column];
        return result;
    }

    Vec3 ToRotationXYZ(const RotationMatrix& matrix)
    {
        const float y = std::asin(std::clamp(-matrix[2][0], -1.0f, 1.0f));
        const float cosY = std::cos(y);
        if (std::abs(cosY) > 0.00001f)
        {
            return {
                std::atan2(matrix[2][1], matrix[2][2]),
                y,
                std::atan2(matrix[1][0], matrix[0][0])
            };
        }

        return { std::atan2(-matrix[1][2], matrix[1][1]), y, 0.0f };
    }
}

void ModelObject::Init()
{
    if(!Load()) 
    {
		// 初期化に失敗した場合はデフォルト値を設定
        position = {};
        scale = { 1.0f, 1.0f, 1.0f };
        rotation = {};
	}
}

bool ModelObject::Load(const std::string& filePath)
{
	// モデルキーが空の場合は読み込みをスキップ
    if (modelKey.empty()) return false;

	// JSONファイルを読み込み、オブジェクトの位置、スケール、回転を設定
    try
    {
		// JSONファイルを開く
        std::ifstream file(filePath);
        if (!file) return false;

		// JSONを解析
        nlohmann::json json;
        file >> json;

		// "objects"キーが存在し、オブジェクトが存在するか確認
        const auto objects = json.find("objects");
        if (objects == json.end() || !objects->is_object()) return false;
        const auto saved = objects->find(NarrowModelKey(modelKey));
        if (saved == objects->end() || !saved->is_object()) return false;

		// JSONからVec3を読み込むためのラムダ関数
        const auto readVector = [](const nlohmann::json& value, Vec3& output)
        {
            if (!value.is_array() || value.size() != 3) return false;
            output = { value[0].get<float>(), value[1].get<float>(), value[2].get<float>() };
            return std::isfinite(output.x) && std::isfinite(output.y) && std::isfinite(output.z);
        };

		// JSONから読み込んだ値を一時的に保持し、検証後に設定
        Vec3 loadedPosition = position;
        Vec3 loadedScale = scale;
        Vec3 loadedRotation = rotation;
        if (!readVector(saved->at("position"), loadedPosition) ||
            !readVector(saved->at("scale"), loadedScale) ||
            !readVector(saved->at("rotation"), loadedRotation) ||
            loadedScale.x <= 0.0f || loadedScale.y <= 0.0f || loadedScale.z <= 0.0f)
            return false;

		// すべての値が有効であれば、オブジェクトのプロパティを更新
        position = loadedPosition;
        scale = loadedScale;
        rotation = loadedRotation;
        return true;
    }
    catch (const std::exception&)
    {
		// 万が一、JSONの読み込みや解析に失敗した場合はfalseを返す
        return false;
    }
}

bool ModelObject::Save(const std::string& filePath) const
{
	// モデルキーが空、または位置・スケール・回転のいずれかが無限大やNaNの場合は保存をスキップ
	// スケールが0以下の場合も保存をスキップ
    // std::isfiniteは、引数が有限の値かどうか判定する関数
    if (modelKey.empty() ||
        !std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z) ||
        !std::isfinite(scale.x) || !std::isfinite(scale.y) || !std::isfinite(scale.z) ||
        !std::isfinite(rotation.x) || !std::isfinite(rotation.y) || !std::isfinite(rotation.z) ||
        scale.x <= 0.0f || scale.y <= 0.0f || scale.z <= 0.0f)
        return false;

	// JSONファイルにオブジェクトの位置、スケール、回転を保存
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

        json["objects"][NarrowModelKey(modelKey)] = {
            { "position", { position.x, position.y, position.z } },
            { "scale", { scale.x, scale.y, scale.z } },
            { "rotation", { rotation.x, rotation.y, rotation.z } }
        };

        const std::filesystem::path path(filePath);
        if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path());
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

void ModelObject::ApplyTransform(Vec3 masterPos) const
{
    const int handle = RM().GetModel(modelKey);
    if (handle < 0) return;

    const Vec3 modelPosition = position + masterPos - RotateXYZ(center, rotation);

    MV1SetPosition(handle, DxConv::ToVECTOR(modelPosition));
    MV1SetScale(handle, DxConv::ToVECTOR(scale));
    MV1SetRotationXYZ(handle, DxConv::ToVECTOR(rotation));
}

void ModelObject::Draw(Vec3 masterPos) const
{
    const int handle = RM().GetModel(modelKey);
    if (handle < 0) return;

    ApplyTransform(masterPos);
    ApplyCollisionColor();
    MV1DrawModel(handle);
}

ModelObject ModelObject::WithParentTransform(const Vec3& parentPosition, const Vec3& parentRotation) const
{
    ModelObject world = *this;
    world.position = parentPosition + RotateXYZ(position, parentRotation);
    world.rotation = ToRotationXYZ(Multiply(MakeRotationMatrix(parentRotation), MakeRotationMatrix(rotation)));
    return world;
}

void ModelObject::FadeTo(const ActorBase& player, float speed, float deltaTime)
{
    if (player.GetPosition().y >= position.y)
    {
        SetAlpha(1.0f);
        return;
    }

    const float targetAlpha = 0.0f;
    const float step = std::max(speed, 0.0f) * std::max(deltaTime, 0.0f);
    const float difference = targetAlpha - alpha;
    if (std::abs(difference) <= step)
    {
        SetAlpha(targetAlpha);
        return;
    }
    SetAlpha(alpha + (difference > 0.0f ? step : -step));
}




Collision::Ellipsoid ModelObject::GetHitEllipsoid() const
{
     int handle = RM().GetModel(modelKey);
    if (handle < 0) return {};


    ApplyTransform();
    if (MV1SetupReferenceMesh(handle, -1, TRUE, TRUE) < 0) return {};//キャラがないとき
	// 参照用メッシュを更新して取得
    const int refreshResult = MV1RefreshReferenceMesh(handle, -1, TRUE, TRUE);
    const auto mesh = MV1GetReferenceMesh(handle, -1, TRUE, TRUE);
	// 頂点の最小座標と最大座標を取得
    const Vec3 minimum = DxConv::ToVec3(mesh.MinPosition);
    const Vec3 maximum = DxConv::ToVec3(mesh.MaxPosition);
	// 頂点がない場合は無効とする
	const bool valid = refreshResult >= 0 && mesh.VertexNum > 0;    // 頂点がない場合は無効とする
	DxLib::MV1TerminateReferenceMesh(handle, -1, TRUE, TRUE);//参照用メッシュの後始末

    if (!valid) return {};

	return { (minimum + maximum) * 0.5f, (maximum - minimum) * 0.5f };//中心と半径を返す
	//return { position, GetSize() * 0.5f }; // 中心と半径を返す
}

void ModelObject::ApplyCollisionColor() const
{
    const int handle = RM().GetModel(modelKey);
    if (handle < 0) return;

	const auto color = collisionHighlighted //カラーを赤にするか白にするか
        ? GetColorF(1.0f, 0.0f, 0.0f, 1.0f)
        : GetColorF(1.0f, 1.0f, 1.0f, 1.0f);

    MV1SetDifColorScale(handle, color);     //ディフューズ色。光が当たった面の基本的な色  だってよ　　　基本これでよき
    MV1SetAmbColorScale(handle, color);     //アンビエント色。環境光による色            だってよ      基本これでよき
    MV1SetSpcColorScale(handle, color);     //スペキュラ色。光沢・ハイライトの色         だってよ     これは正直いらない
    MV1SetEmiColorScale(handle, color);     //エミッシブ色。自己発光部分の色                        これは正直いらない
}
namespace Collision
{
    bool UpdateModelContact(ModelObject& a, ModelObject& b)
	{//二つのモデルの当たり判定を行い、衝突している場合はcollisionHighlightedをtrueにする
        const bool touching = IsHitEllipsoidEllipsoid(a.GetHitEllipsoid(), b.GetHitEllipsoid());
        a.collisionHighlighted = touching;
        b.collisionHighlighted = touching;
        return touching;
    }
}
