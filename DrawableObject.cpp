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

    const Vec3 scaledCenter{
        center.x * scale.x,
        center.y * scale.y,
        center.z * scale.z
    };
    const Vec3 modelPosition = position + masterPos - RotateXYZ(scaledCenter, rotation);

    MV1SetPosition(handle, DxConv::ToVECTOR(modelPosition));
    MV1SetScale(handle, DxConv::ToVECTOR(scale));
    MV1SetRotationXYZ(handle, DxConv::ToVECTOR(rotation));
    MV1DrawModel(handle);
}
