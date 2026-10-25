#include "ModelCollision.h"
#include "DrawableObject.h"
#include "ResourceManager.h"
#include "DxConv.h"
#include "DxLib.h"
#include <algorithm>
#include <cmath>

ModelCollision::~ModelCollision()
{
    Clear();
}

void ModelCollision::AddModel(const ModelObject& model, Surface surface)
{
    // 同じモデルでも種類ごとに別登録し、同じ種類の重複だけを除く。
    for (auto& entry : models_)
    {
        if (entry.model == &model && entry.surface == surface)
        {
            return;
        }
    }

    models_.push_back({ &model, surface });
}


void ModelCollision::Clear()
{
    for (const auto& entry : models_)
    {
        if (entry.handle >= 0) MV1DeleteModel(entry.handle);
    }

    models_.clear();

}

bool ModelCollision::Refresh()
{
    bool ready = true;

    for (auto& entry : models_)
    {
        const int source = RM().GetModel(entry.model->GetModelKey());

        if (source != entry.sourceHandle || source < 0)//古いモデル
        {
            if (entry.handle >= 0) {MV1DeleteModel(entry.handle);}//新しいのに入れ替え

            entry.handle = -1;
            entry.sourceHandle = source;
        
        }
        if (source < 0)
        {
            ready = false;
            continue;
        }

        // 親の位置・回転も含め、描画と同じ変換を判定用モデルへコピーする。
        entry.model->ApplyTransform();

        const MATRIX world = MV1GetMatrix(source);


        if (entry.handle < 0)
        {
            entry.handle = MV1DuplicateModel(source);

            if (entry.handle >= 0)
            {
                MV1SetMatrix(entry.handle, world);

                if (MV1SetupCollInfo(entry.handle, -1, 8, 8, 8) < 0)
                {
                    MV1DeleteModel(entry.handle);
                    entry.handle = -1;
                }
            }
            if (entry.handle < 0) ready = false;
            continue;
        }

        // 配置が変わったときだけ、重いメッシュの再計算を行う。
        const MATRIX previous = MV1GetMatrix(entry.handle);
        bool changed = false;

		for (int row = 0; row < 4; ++row)// 4x4行列の各要素を比較して、変化があればフラグを立てる
            for (int column = 0; column < 4; ++column)
                changed |= previous.m[row][column] != world.m[row][column];


        if (changed)
        {
			MV1SetMatrix(entry.handle, world);// 位置・回転・拡大率を反映する
            if (MV1RefreshCollInfo(entry.handle, -1) < 0)
            {
                MV1DeleteModel(entry.handle);
                entry.handle = -1;
                ready = false;
            }
        }
    }
    return ready;
}

bool ModelCollision::IsTouchingWalls(const Vec3& feetPosition, const ModelOb& modelOb) const
// すべての登録モデルに対して、体のカプセルが接触しているかを判定する
{
	if (!modelOb.IsValid()) return false;//体の値が不であれば判定しない

    const Vec3 start = feetPosition + Vec3::Up() * modelOb.radius;
    const Vec3 end = feetPosition + Vec3::Up() * (modelOb.height - modelOb.radius);

    for (const auto& entry : models_)
    {
        if (entry.handle < 0 || entry.surface == Surface::Floor) continue;
		// 壁や登れる壁の判定は体のカプセルとモデルの衝突を調べる
        const auto result = MV1CollCheck_Capsule(entry.handle, -1,
            DxConv::ToVECTOR(start), DxConv::ToVECTOR(end), modelOb.radius);

        const bool hit = result.HitNum > 0;
		MV1CollResultPolyDimTerminate(result);// 判定結果の配列は、当たった場合も当たらない場合も解放する
        if (hit) return true;
    }
    return false;
}

bool ModelCollision::Raycast(const Vec3& start, const Vec3& end, Physics::RayHit& hit) const
{
    return RaycastModels(start, end, hit, false);
}

// 対象のモデルに絞ってから、最も近い衝突を選ぶ。
bool ModelCollision::RaycastModels(const Vec3& start, const Vec3& end, Physics::RayHit& hit,
    bool forLanding, bool ignorePillars) const
{
    bool found = false;

    for (const auto& entry : models_)
    {
        if (entry.handle < 0) continue;
        if (forLanding && entry.surface == Surface::Wall) continue;
        // 柱は接触のTrue/Falseだけを調べ、着地・頭上の位置補正には使わない。
        if (ignorePillars && entry.surface == Surface::Pillar) continue;

        Physics::RayHit modelHit{};
        
        if (!Physics::Raycast(entry.handle, start, end, modelHit)) continue;
        
		if (!found || modelHit.distance < hit.distance)// 近い衝突を優先する
        {
            hit = modelHit;
            found = true;
        }
    }
    return found;
}

bool ModelCollision::RaycastDown(const Vec3& origin, float maxDistance, Physics::RayHit& hit) const
{
    if (maxDistance <= 0.0f) return false;
    // return RaycastModels(origin, origin - Vec3::Up() * maxDistance, hit, true);
  
    return RaycastModels(origin, origin - Vec3::Up() * maxDistance, hit, true, true);
}

bool ModelCollision::RaycastUp(const Vec3& origin, float maxDistance, Physics::RayHit& hit) const
{
    if (maxDistance <= 0.0f) return false;
    // return RaycastModels(origin, origin + Vec3::Up() * maxDistance, hit, false);
   
    return RaycastModels(origin, origin + Vec3::Up() * maxDistance, hit, false, true);
}

Vec3 ModelCollision::GetSize(const ModelObject& model) const
{
    for (const auto& entry : models_)
    {
        if (entry.model != &model || entry.handle < 0) continue;
        // コリジョンが使う参照メッシュを解放しないよう、サイズ取得は別の複製で行う。
        const int sizeHandle = MV1DuplicateModel(entry.handle);

        if (sizeHandle < 0) return {};

        MV1SetMatrix(sizeHandle, MV1GetMatrix(entry.handle));
		//Martixで位置・回転・拡大率を反映する

		if (MV1SetupReferenceMesh(sizeHandle, -1, TRUE, TRUE) < 0)// 参照用メッシュのセットアップに失敗した場合は、サイズを返さずに解放する
        {
            MV1DeleteModel(sizeHandle);
            return {};
        }
        const int refreshed = MV1RefreshReferenceMesh(sizeHandle, -1, TRUE, TRUE);
        const auto mesh = MV1GetReferenceMesh(sizeHandle, -1, TRUE, TRUE);

        Vec3 size{};

        if (refreshed >= 0 && mesh.VertexNum > 0)
            size = DxConv::ToVec3(mesh.MaxPosition) - DxConv::ToVec3(mesh.MinPosition);

		MV1TerminateReferenceMesh(sizeHandle, -1, TRUE, TRUE);// 参照用メッシュの後始末を行う
		MV1DeleteModel(sizeHandle);                             // 複製モデルを解放する
        return size;
    }
    return {};
}

bool ModelCollision::FindWall(const Vec3& feetPosition, const ModelOb& modelOb, Vec3& wallNormal,
    const Vec3& moveDirection, float* wallTopY, float detectionMargin, float topHoldDistance) const
	// 体のカプセルの側面に接触している壁を探す。見つかれば法線を返す。
{
    wallNormal = {};
    if (wallTopY) *wallTopY = 0.0f;

    if (!modelOb.IsValid()) return false;
    const bool keepTopContact = topHoldDistance > 0.0f;

    // 移動の衝突判定より少し広い範囲で側面を拾う。体の高さは変えない。
    const float detectionRadius = modelOb.radius + (std::max)(0.0f, detectionMargin);

    // 上端で接触を維持するときだけ、足元より下まで探す。
    const float lowerReach = keepTopContact ? topHoldDistance : 0.0f;
    const Vec3 start = feetPosition + Vec3::Up() * (modelOb.radius - lowerReach);
    const Vec3 end = feetPosition + Vec3::Up() * (modelOb.height - modelOb.radius);
    const float headY = feetPosition.y + modelOb.height;

    const Vec3 direction = Vec3(moveDirection.x, 0.0f, moveDirection.z).Normalized();
    bool foundWall = false;
    float bestDirection = 2.0f;
    float bestDistance = detectionRadius;

    for (const auto& entry : models_)
    {
        if (entry.handle < 0 || entry.surface != Surface::Pillar) continue;
        const int handle = entry.handle;

        const auto result = MV1CollCheck_Capsule(handle, -1,
            DxConv::ToVECTOR(start),
            DxConv::ToVECTOR(end),
            detectionRadius);

        for (int i = 0; i < result.HitNum; ++i)
        {
            const auto& poly = result.Dim[i];
            const Vec3 normal = DxConv::ToVec3(poly.Normal).Normalized();

            // 向きがない面や、上下を向いた面・斜面は使わない。
            if (normal.LengthSq() == 0.0f || std::abs(normal.y) > 0.01f)
            {
                continue;
            }

            float top = (std::max)({ poly.Position[0].y, poly.Position[1].y, poly.Position[2].y });
            float bottom = (std::min)({ poly.Position[0].y, poly.Position[1].y, poly.Position[2].y });


            // 足元が上端付近に達しているか
            const bool atTop = feetPosition.y >= top - modelOb.skin;
            if (atTop)
            {
                // 通常時は、この側面を壁として扱わない。
                if (!keepTopContact)
                {
                    continue;
                }

                // 登り中も、上端より完全に上なら側面を使わない。
                const bool aboveTop = feetPosition.y > top + modelOb.skin;
                if (aboveTop)
                {
                    continue;
                }
            }

            // 体の高さと重ならない側面は使わない。
            if (headY <= bottom) continue;

            Vec3 outward = Vec3(normal.x, 0.0f, normal.z).Normalized();
            const float signedDistance = Vec3::Dot(feetPosition - DxConv::ToVec3(poly.Position[0]), outward);
            // モデルの面の表裏によらず、壁から移動する物体へ向く法線にそろえる。
            if (signedDistance < 0.0f) outward = -outward;

            const float wallDirection = Vec3::Dot(direction, outward);
            const float distance = std::abs(signedDistance);
            // 角で複数の面に触れた場合も、入力が向かっている面を優先する。
            if (!foundWall || wallDirection < bestDirection - 0.0001f ||
                (std::abs(wallDirection - bestDirection) <= 0.0001f &&
                    distance < bestDistance))
            {
				wallNormal = outward;           // 壁の法線は、プレイヤーの進行方向に向かうようにそろえる。
				bestDirection = wallDirection;  // 壁の法線と進行方向の内積が最も小さい面を優先する。
                bestDistance = distance;        // 壁までの距離が最も近い面を優先する。
                if (wallTopY) *wallTopY = top;
				foundWall = true;               //  壁の法線が見つかったことを記録する。
            }
        }

        // 判定結果を解放してから、見つかったかどうかを返す。
        MV1CollResultPolyDimTerminate(result);
    }
    return foundWall;
}

Vec3 ModelCollision::SlideAlongWalls(const Vec3& oldPosition, const Vec3& desiredPosition,
    const ModelOb& modelOb, bool& touched) const
{
   
    Vec3 move{ desiredPosition.x - oldPosition.x, 0.0f, desiredPosition.z - oldPosition.z };
    Vec3 candidate = desiredPosition;
    touched = false;

    if (!modelOb.IsValid()) return desiredPosition;
    // 隣接する柱の側面も考慮して、数回判定し直す。移動速度の増加や進行方向の自動補正は行わない。
    for (int attempt = 0; attempt < 4; ++attempt)
    {
        // 移動予定位置に、足元基準のカプセルを置いて調べる。
        const Vec3 start = candidate + Vec3::Up() * modelOb.radius;
        const Vec3 end = candidate + Vec3::Up() * (modelOb.height - modelOb.radius);

        Vec3 wallNormal{};
        float mostInward = -0.0001f;

        for (const auto& entry : models_)
        {
            // 柱への押し戻しを停止する。IsTouchingWalls / FindWall の真偽判定は残す。
            // if (entry.handle < 0 || entry.surface == Surface::Floor) continue;
            if (entry.handle < 0 || entry.surface != Surface::Wall) continue;
            const int handle = entry.handle;

            const auto result = MV1CollCheck_Capsule(handle, -1,
                DxConv::ToVECTOR(start), DxConv::ToVECTOR(end), modelOb.radius);

            for (int i = 0; i < result.HitNum; ++i)
            {
                const auto& poly = result.Dim[i];
                Vec3 normal = DxConv::ToVec3(poly.Normal).Normalized();

                if (std::abs(normal.y) > 0.01f) continue; // 柱の上下面と斜面は対象外にする。

                float top = (std::max)(poly.Position[0].y,
                    (std::max)(poly.Position[1].y, poly.Position[2].y));

                if (candidate.y >= top - modelOb.skin) continue; // 柱の上にいる場合は側面との衝突を処理しない。

                normal.y = 0.0f;
                normal = normal.Normalized();
                // 部屋の内側でも使えるように、移動前の位置へ法線を向ける。
                if (Vec3::Dot(oldPosition - DxConv::ToVec3(poly.Position[0]), normal) < 0.0f)
                    normal = -normal;
				//壁に向かう移動の際に、最も壁に向かう法線を採用する。
                float inward = Vec3::Dot(move, normal);

                if (inward < mostInward)
                { mostInward = inward; wallNormal = normal; }
            }
            MV1CollResultPolyDimTerminate(result);
        }
        if (wallNormal.LengthSq() == 0.0f) return candidate; // 壁から離れる移動も含む。
        touched = true;

        // 壁へ入り込む成分だけを取り除き、横方向に弾き飛ばさない。
        move -= wallNormal * Vec3::Dot(move, wallNormal);
        candidate = { oldPosition.x + move.x, desiredPosition.y, oldPosition.z + move.z };
    }
    // 最後の再判定でも移動できなければ、このフレームの移動前の水平位置を維持する。
    return { oldPosition.x, desiredPosition.y, oldPosition.z };
}


