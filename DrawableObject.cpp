#include "DrawableObject.h"
#include "DxConv.h"
#include "ResourceManager.h"
#include <cmath>

namespace
{
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

void ModelObject::Draw(Vec3 masterPos) const
{
    const int handle = RM().GetModel(modelKey);
    if (handle < 0) return;

    const Vec3 modelPosition = position + masterPos - RotateXYZ(center, rotation);

    MV1SetPosition(handle, DxConv::ToVECTOR(modelPosition));
    MV1SetScale(handle, DxConv::ToVECTOR(scale));
    MV1SetRotationXYZ(handle, DxConv::ToVECTOR(rotation));
    ApplyCollisionColor();
    MV1DrawModel(handle);
}

Collision::Ellipsoid ModelObject::GetHitEllipsoid() const
{
     int handle = RM().GetModel(modelKey);
    if (handle < 0) return {};


    MV1SetPosition(handle, DxConv::ToVECTOR(position - RotateXYZ(center, rotation)));
    MV1SetScale(handle, DxConv::ToVECTOR(scale));
    MV1SetRotationXYZ(handle, DxConv::ToVECTOR(rotation));
    if (MV1SetupReferenceMesh(handle, -1, TRUE, TRUE) < 0) return {};//キャラがないとき
	// 参照用メッシュを更新して取得
    const int refreshResult = MV1RefreshReferenceMesh(handle, -1, TRUE, TRUE);
    const auto mesh = MV1GetReferenceMesh(handle, -1, TRUE, TRUE);
    const Vec3 minimum = DxConv::ToVec3(mesh.MinPosition);
    const Vec3 maximum = DxConv::ToVec3(mesh.MaxPosition);
	const bool valid = refreshResult >= 0 && mesh.VertexNum > 0;    // 頂点がない場合は無効とする
	MV1TerminateReferenceMesh(handle, -1, TRUE, TRUE);//参照用メッシュの後始末
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
    void UpdateModelContact(ModelObject& a, ModelObject& b)
	{//二つのモデルの当たり判定を行い、衝突している場合はcollisionHighlightedをtrueにする
        const bool touching = IsHitEllipsoidEllipsoid(a.GetHitEllipsoid(), b.GetHitEllipsoid());
        a.collisionHighlighted = touching;
        b.collisionHighlighted = touching;
    }
}
