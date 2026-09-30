#include "DrawableObject.h"
#include "DxConv.h"
#include "ResourceManager.h"

void ImageObject::Draw() const
{
    const auto* sprite = RM().GetSprite(imageKey);
    if (!sprite) return;

    sprite->Draw(position, scale, rotation);
}

void ModelObject::Draw() const
{
    const int handle = RM().GetModel(modelKey);
    if (handle < 0) return;

    MV1SetPosition(handle, DxConv::ToVECTOR(position));
    MV1SetScale(handle, DxConv::ToVECTOR(scale));
    MV1SetRotationXYZ(handle, DxConv::ToVECTOR(rotation));
    MV1DrawModel(handle);
}
