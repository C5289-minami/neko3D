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
            entry.boundsReady = false;
        
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
            else if (entry.surface == Surface::Pillar && !UpdatePillarBounds(entry)) ready = false;
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
			entry.boundsReady = false;
			MV1SetMatrix(entry.handle, world);// 位置・回転・拡大率を反映する
            if (MV1RefreshCollInfo(entry.handle, -1) < 0)
            {
                MV1DeleteModel(entry.handle);
                entry.handle = -1;
                ready = false;
            }
        }
        if (entry.handle >= 0 && entry.surface == Surface::Pillar &&
            !entry.boundsReady && !UpdatePillarBounds(entry)) ready = false;
    }
    return ready;
}

bool ModelCollision::UpdatePillarBounds(Entry& entry)
{
    const auto bounds = entry.model->GetHitEllipsoid();
    //中心点でなければ使えないのです　
    entry.boundsReady = bounds.radii.x > 0.0f && bounds.radii.y > 0.0f && bounds.radii.z > 0.0f;
 

    if (entry.boundsReady)
    {
        entry.center = bounds.center;
        entry.radii = bounds.radii;
        entry.topY = bounds.center.y + bounds.radii.y;
    }
    return entry.boundsReady;
}

bool ModelCollision::GetPillarBounds(const ModelObject* model, Vec3& center, float& topY) const
{
    for (const auto& entry : models_)
    {
        if (entry.model != model || entry.surface != Surface::Pillar ||
            entry.handle < 0 || !entry.boundsReady) continue;
        center = entry.center;
        topY = entry.topY;
        return true;
    }
    return false;
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

bool ModelCollision::RaycastPillarTop(const Vec3& origin,
            float maxDistance, float topOffset,Physics::RayHit& hit) const
{
    if (maxDistance <= 0.0f) return false;
    const Vec3 offset = Vec3::Up() * topOffset;
    const Vec3 start = origin + offset;
    const Vec3 end = start - Vec3::Up() * maxDistance;
    bool found = false;

    for (const auto& entry : models_)
    {
        if (entry.handle < 0 || entry.surface != Surface::Pillar) continue;

        Physics::RayHit modelHit{};

        if (!Physics::Raycast(entry.handle, start, end, modelHit) || modelHit.normal.y <= 0.5f) continue;
        modelHit.point -= offset;
		//モデルが複数あっても、最も近い衝突を優先する
        if (!found || modelHit.distance < hit.distance)
        {
            hit = modelHit;
            found = true;
        }
    }
    return found;
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
    const Vec3& moveDirection, float* wallTopY, float detectionMargin, float topHoldDistance,
    Vec3* pillarCenter, const ModelObject** pillarModel) const
	// 体のカプセルの側面に接触している壁を探す。見つかれば法線を返す。
{
    wallNormal = {};
	if (wallTopY) *wallTopY = 0.0f;          //壁が見つからなかったときの値を返す
	if (pillarCenter) *pillarCenter = {};    //センターが見つからなかったときの値を返す
    if (pillarModel) *pillarModel = nullptr; //ぬるぽ ガッ

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
    const ModelObject* hitPillar = nullptr;

    for (const auto& entry : models_)
    {
        if (entry.handle < 0 || entry.surface != Surface::Pillar) continue;

        // 表面に触れない柱の内部でも、Spaceを押し直して上昇を再開できる。
        if (entry.boundsReady && feetPosition.y < entry.topY - modelOb.skin &&
            headY > entry.center.y - entry.radii.y)
        {
            const float x = (feetPosition.x - entry.center.x) / entry.radii.x;
            const float z = (feetPosition.z - entry.center.z) / entry.radii.z;
            if (x * x + z * z <= 1.0f)
            {
                wallNormal = Vec3(feetPosition.x - entry.center.x, 0.0f,
                    feetPosition.z - entry.center.z).Normalized();
                if (wallTopY) *wallTopY = entry.topY;
                hitPillar = entry.model;
                foundWall = true;
                break;
            }
        }
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
				hitPillar = entry.model;
				foundWall = true;               //  壁の法線が見つかったことを記録する。
            }
        }

        // 判定結果を解放してから、見つかったかどうかを返す。
        MV1CollResultPolyDimTerminate(result);
    }

    // 接触中の柱そのものの中心を返す。親の配置・拡大率も反映される。
    if (hitPillar && pillarCenter)
    {
       /* const auto bounds = hitPillar->GetHitEllipsoid();
        if (bounds.radii.x <= 0.0f || bounds.radii.z <= 0.0f) return false;
        *pillarCenter = bounds.center;*/

        float topY{};
        if (!GetPillarBounds(hitPillar, *pillarCenter, topY)) return false;
        if (wallTopY) *wallTopY = topY;
    }
    if (pillarModel) *pillarModel = hitPillar;
    return foundWall;
}

bool ModelCollision::RaycastWall(const Vec3& start, const Vec3& end, Physics::RayHit& hit) const
{
    bool found = false;
    for (const auto& entry : models_)
    {
        if (entry.handle < 0 || entry.surface == Surface::Pillar) continue;

        Physics::RayHit candidate{};
        if (!Physics::Raycast(entry.handle, start, end, candidate)) continue;
        // 足場の上面・下面と、歩ける傾斜は横移動を止めない。
        if (std::abs(candidate.normal.y) >= 0.5f) continue;

        // 部屋の内側から当たる場合も、レイの開始側へ法線を向ける。
        if (Vec3::Dot(end - start, candidate.normal) > 0.0f)
            candidate.normal = -candidate.normal;

        if (!found || candidate.distance < hit.distance)
        {
            hit = candidate;
            found = true;
        }
    }
    return found;
}

Vec3 ModelCollision::SlideAlongWalls(const Vec3& oldPosition, const Vec3& desiredPosition,
    const ModelOb& modelOb, bool& touched) const
{
    touched = false;
    if (!modelOb.IsValid()) return desiredPosition;

        //水平移動
    Vec3 position{ oldPosition.x, desiredPosition.y, oldPosition.z };
    Vec3 remaining{ desiredPosition.x - oldPosition.x, 0.0f, desiredPosition.z - oldPosition.z };
    const float rayHeight = modelOb.height / 2.0f; // 横向きレイを出す、体の中央の高さ。

    for (int attempt = 0; attempt < 4; ++attempt)
    {
        float length = remaining.Length();

        if (length <= 0.00001f) break;

		Vec3 direction = remaining / length;            // ベクトルの正規化。進行方向を求める。
		Vec3 right{ direction.z, 0.0f, -direction.x };  // 進行方向の右向きベクトルを作る
		Vec3 center = position + Vec3::Up() * rayHeight;// 体の中央の高さで、左右にレイを飛ばす

        float travel = length;

        Vec3 wallNormal{};

        for (int side = -1; side <= 1; ++side)
        {
            const Vec3 start = center + right * (modelOb.radius * side);
            const Vec3 end = start + direction * (length + modelOb.radius + modelOb.skin);
   
            Physics::RayHit hit{};
            
			if (!RaycastWall(start, end, hit)) continue;// 壁に当たらなければ
            
			float approaching = -Vec3::Dot(direction, hit.normal);// 進行方向と壁の法線の内積が負なら、壁に向かっている。
            
            if (approaching <= 0.00001f) continue;


            // 体の幅とskin分は壁から離す。傾いた側面では体の長さも考慮する。
            float clearance = modelOb.radius + modelOb.skin;
            clearance += (rayHeight - modelOb.radius) * std::abs(hit.normal.y);
			//法線とない席の処理で、壁の面からどれだけ離れているかを計算する。
            const float gap = Vec3::Dot(center - hit.point, hit.normal) - clearance;
            float allowed = gap / approaching;
            
            if (allowed < 0.0f) allowed = 0.0f;
            //壁にぶつかったら、かべっどまり
            if (allowed <= travel)
            {
                travel = allowed;
                wallNormal = Vec3{ hit.normal.x, 0.0f, hit.normal.z }.Normalized();
            }
		}

        position += direction * travel;//進行方向じぇ
     
        touched = true;
		remaining -= direction * travel;//残りの移動量を更新する
      
        // 速度全体をゼロにせず、壁へ入り込む成分だけを除く。
        const float inward = Vec3::Dot(remaining, wallNormal);
      
        if (inward < 0.0f) remaining -= wallNormal * inward;
    }
    return position;
}

