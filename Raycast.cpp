// =============================
// Gameplay/Physics/Raycast.cpp
// =============================
#include "Raycast.h"
#include "DxLib.h"
#include "DxConv.h"

bool Physics::Raycast(int modelHandle, Vec3 start, Vec3 end, RayHit& hit)
{
    MV1_COLL_RESULT_POLY result = DxLib::MV1CollCheck_Line(
        modelHandle, -1, DxConv::ToVECTOR(start), DxConv::ToVECTOR(end));

    if (result.HitFlag == 0) return false;  // 光線は当たっていない

    Vec3 hitPos = DxConv::ToVec3(result.HitPosition);
    hit.point = hitPos;
    hit.normal = DxConv::ToVec3(result.Normal).Normalized();
    hit.distance = (hitPos - start).Length();

    // 光線が当たった
    return true;
}

bool Physics::Raycast(const std::vector<int>& modelHandles, Vec3 start, Vec3 end, RayHit& hit)
{
    bool isHit = false;
    RayHit nearestHit{};

    for (int modelHandle : modelHandles)
    {
        if (modelHandle < 0) continue;

        RayHit modelHit{};
        // 上にある元の処理を、各モデルに対してそのまま呼ぶ。
        if (!Raycast(modelHandle, start, end, modelHit)) continue;

        if (!isHit || modelHit.distance < nearestHit.distance)
        {
            nearestHit = modelHit;
            isHit = true;
        }
    }

    if (isHit)
    {
        hit = nearestHit;
    }
    return isHit;
}

bool Physics::RaycastDown(const std::vector<int>& modelHandles, Vec3 origin, float maxDistance, RayHit& hit)
{
    if (maxDistance <= 0.0f) return false;
    return Raycast(modelHandles, origin, origin - Vec3::Up() * maxDistance, hit);
}

bool Physics::RaycastUp(const std::vector<int>& modelHandles, Vec3 origin, float maxDistance, RayHit& hit)
{
    if (maxDistance <= 0.0f) return false;
    return Raycast(modelHandles, origin, origin + Vec3::Up() * maxDistance, hit);
}
