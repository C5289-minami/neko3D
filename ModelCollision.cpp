#include "ModelCollision.h"
#include "DrawableObject.h"
#include "ResourceManager.h"
#include "DxConv.h"
#include "DxLib.h"
#include <algorithm>
#include <cmath>
#include "Consts.h"

namespace
{
	constexpr float FloorClearance = 0.01f;
	constexpr float FloorContactTolerance = 0.001f;
}

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

void ModelCollision::AddFallRecoveryFloor(const ModelObject& model)
{
	AddModel(model, Surface::Floor);
	fallRecoveryFloor_ = &model;
}

void ModelCollision::Clear()
{
	for (const auto& entry : models_)
	{
		if (entry.handle >= 0) MV1DeleteModel(entry.handle);
	}

	models_.clear();
	fallRecoveryFloor_ = nullptr;
	contactCapsule_ = {};

}

bool ModelCollision::IsTouchingCapsule(const Collision::Sphere& sphere) const
{
    return contactCapsule_.radius > 0.0f && sphere.radius > 0.0f &&
        Collision::IsHitCapsuleSphere(contactCapsule_, sphere);
}

bool ModelCollision::Refresh()
{
	bool ready = true;

	for (auto& entry : models_)
	{
		const bool needsBounds = entry.surface == Surface::Pillar || entry.surface == Surface::Stand ||
			(entry.surface == Surface::Floor && entry.model == fallRecoveryFloor_);
		const int source = RM().GetModel(entry.model->GetModelKey());

		if (source != entry.sourceHandle || source < 0)//古いモデル
		{
			if (entry.handle >= 0) { MV1DeleteModel(entry.handle); }//新しいのに入れ替え

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
			else if (needsBounds && !UpdateBounds(entry)) ready = false;
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
		if (entry.handle >= 0 && needsBounds &&
			!entry.boundsReady && !UpdateBounds(entry)) ready = false;
	}
	return ready;
}

bool ModelCollision::UpdateBounds(Entry& entry)
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

	Vec3 normal{};
	if (FindStandSide(feetPosition, modelOb, feetPosition, {}, normal)) return true;

	for (const auto& entry : models_)
	{
		if (entry.handle < 0 || entry.surface == Surface::Floor || entry.surface == Surface::Stand) continue;
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
	bool found = false;

	for (const auto& entry : models_)
	{
		if (entry.handle < 0) continue;

		Physics::RayHit modelHit{};
		if (!Physics::Raycast(entry.handle, start, end, modelHit)) continue;

		if (!found || modelHit.distance < hit.distance)
		{
			hit = modelHit;
			found = true;
		}
	}
	return found;
}

std::vector<ModelObject*> ModelCollision::GetModelsHitByRay(
	const Vec3& start,
	const Vec3& end) const
{
	std::vector<ModelObject*> hitModels;

	for (const auto& entry : models_)
	{
		if (entry.handle < 0) continue;

		Physics::RayHit hit{};

		if (!Physics::Raycast(entry.handle, start, end, hit))
			continue;

		ModelObject* model = const_cast<ModelObject*>(entry.model);

		if (std::find(hitModels.begin(), hitModels.end(), model)
			== hitModels.end())
		{
			hitModels.push_back(model);
		}
	}

	return hitModels;
}

// 対象のモデルに絞ってから、最も近い衝突を選ぶ。
bool ModelCollision::RaycastModels(const Vec3& start, const Vec3& end, Physics::RayHit& hit,
	bool forLanding, bool ignorePillars, bool* hitRecoveryFloor, bool includeRecoveryFloor) const
{
	bool found = false;

	for (const auto& entry : models_)
	{
		if (entry.handle < 0) continue;
		if (forLanding && entry.surface == Surface::Wall) continue;
		const bool isRecoveryFloor = entry.model == fallRecoveryFloor_ && entry.surface == Surface::Floor;
		if (!includeRecoveryFloor && isRecoveryFloor) continue;
		// 柱は接触のTrue/Falseだけを調べ、着地・頭上の位置補正には使わない。
		if (ignorePillars && entry.surface == Surface::Pillar) continue;

		Physics::RayHit modelHit{};

		if (!Physics::Raycast(entry.handle, start, end, modelHit)) continue;

		if (!found || modelHit.distance < hit.distance)// 近い衝突を優先する
		{
			hit = modelHit;
			if (hitRecoveryFloor)
				*hitRecoveryFloor = isRecoveryFloor;
			found = true;
		}
	}
	return found;
}

bool ModelCollision::RaycastDown(const Vec3& origin, float maxDistance, Physics::RayHit& hit,
	bool* hitRecoveryFloor, bool includeRecoveryFloor) const
{
	if (hitRecoveryFloor) *hitRecoveryFloor = false;
	if (maxDistance <= 0.0f) return false;

	return RaycastModels(origin, origin - Vec3::Up() * maxDistance, hit,
		true, false, hitRecoveryFloor, includeRecoveryFloor);
}

const ModelCollision::Entry* ModelCollision::GetFallRecoveryFloor() const	
// fallRecoveryFloor_ が登録されているかを確認し、登録されていればその Entry を返す
{
	for (const auto& entry : models_)
	{
		if (entry.model == fallRecoveryFloor_ && entry.surface == Surface::Floor &&
			entry.handle >= 0 &&entry.boundsReady) return &entry;
	}
	return nullptr;
}

bool ModelCollision::SweepFallRecoveryFloor(const Collision::Sphere& body, const Vec3& movement,
	Physics::RayHit& hit) const
{
	const auto* floor = GetFallRecoveryFloor();
	const float length = movement.Length();
	if (!floor || body.radius <= 0.0f || length <= Const::EPS) return false;

	const Vec3 direction = movement / length;
	const auto triangles = MV1CollCheck_Capsule(floor->handle, -1, DxConv::ToVECTOR(body.center),
		DxConv::ToVECTOR(body.center + movement), body.radius + FloorClearance + FloorContactTolerance);
	float nearest = length;
	bool found = false;
	for (int i = 0; i < triangles.HitNum; ++i)
	{
		const auto& triangle = triangles.Dim[i];
		float travel = 0.0f;
		for (int step = 0; step < 16; ++step)
		{
			const Vec3 center = body.center + direction * travel;
			const VECTOR point = DxConv::ToVECTOR(center);
			TRIANGLE_POINT_RESULT closest{};
			Triangle_Point_Analyse(&triangle.Position[0], &triangle.Position[1], &triangle.Position[2],
				&point, &closest);
			const Vec3 surface = DxConv::ToVec3(closest.Tri_MinDist_Pos);
			const Vec3 separation = center - surface;
			const float distance = separation.Length();
			const Vec3 normal = distance > Const::EPS ? separation / distance :
				DxConv::ToVec3(triangle.Normal).Normalized();
			const float approach = -Vec3::Dot(direction, normal);
			if (approach <= Const::EPS) break;

			// 最近接点の接平面まで進める。面の端や頂点でも球が当たる手前で止まる。
			const float gap = distance - body.radius - FloorClearance;
			if (gap <= FloorContactTolerance || step == 15)
			{
				nearest = travel;
				hit = { surface, normal, travel };
				found = true;
				break;
			}
			travel += gap / approach;
			if (travel > nearest) break;
		}
	}
	MV1CollResultPolyDimTerminate(triangles);
	return found;
}

Vec3 ModelCollision::SlideAlongFallRecoveryFloor(const Collision::Sphere& body, Vec3 movement,
	Vec3& velocity) const
{
	Collision::Sphere movingBody = body;
	for (int attempt = 0; attempt < 4 && movement.LengthSq() > Const::EPS * Const::EPS; ++attempt)
	{
		Physics::RayHit hit{};
		if (!SweepFallRecoveryFloor(movingBody, movement, hit))
			return movingBody.center + movement;

		const Vec3 advance = movement.Normalized() * hit.distance;
		movingBody.center += advance;
		const float clearance = Vec3::Dot(movingBody.center - hit.point, hit.normal);
		if (clearance < body.radius)
			movingBody.center += hit.normal * (body.radius + FloorClearance - clearance);

		movement -= advance;
		movement -= hit.normal * (std::min)(0.0f, Vec3::Dot(movement, hit.normal));
		if (hit.normal.y < 0.0f && velocity.y > 0.0f)
			velocity.y = 0.0f;
	}
	return movingBody.center;
}

bool ModelCollision::HasFallRecoverySupport(const Vec3& position, const ModelOb& body,
	float walkableNormalY) const
{
	const auto* floor = GetFallRecoveryFloor();
	if (!floor || !body.IsValid()) return false;

	// 今の位置を4つコピーして、調べる場所を前後左右に動かす。
	Vec3 checkPositions[] = { position, position, position, position };
	checkPositions[0].x += body.radius; // 右
	checkPositions[1].x -= body.radius; // 左
	checkPositions[2].z += body.radius; // 奥
	checkPositions[3].z -= body.radius; // 手前

	// 床を調べる線が、体の半径と余白のぶんだけ上下に届くようにする。
	const Vec3 rayHeight = Vec3::Up() * (body.radius + body.skin);

	// 上で作った4か所を、1か所ずつ調べる。
	for (const Vec3& checkPosition : checkPositions)
	{
		// 調べる場所の上から下へ、線を飛ばす。
		const Vec3 rayStart = checkPosition + rayHeight;
		const Vec3 rayEnd = checkPosition - rayHeight;
		Physics::RayHit hit{};
		const bool foundFloor = Physics::Raycast(floor->handle, rayStart, rayEnd, hit);

		// 1か所でも床がなければ、この位置を復帰先として記録しない。
		if (!foundFloor)
		{
			return false;
		}

		// 床があっても、立てない傾きなら復帰先として記録しない。
		const bool isWalkable = hit.normal.y >= walkableNormalY;
		if (!isWalkable)
		{
			return false;
		}
	}

	// 4か所すべてに立てる床があるので、今のプレイヤーの位置を記録できる。
	return true;
}

bool ModelCollision::CanLandOnFallRecoveryFloor(const Vec3& position, float skin,
	float snapDistance, float walkableNormalY) const
{
	const auto* floor = GetFallRecoveryFloor();
	if (!floor) return false;

	// 保存した高さから、通常の着地判定が届く範囲に1階の床があるか確認する。
	const Vec3 start = position + Vec3::Up() * skin;
	const Vec3 end = position - Vec3::Up() * snapDistance;
	Physics::RayHit hit{};
	const bool foundFloor = Physics::Raycast(floor->handle, start, end, hit);
	if (!foundFloor)
	{
		return false;
	}

	const bool isWalkable = hit.normal.y >= walkableNormalY;
	return isWalkable;
}

bool ModelCollision::RaycastUp(const Vec3& origin, float maxDistance, Physics::RayHit& hit,
	bool includeRecoveryFloor) const
{
	if (maxDistance <= 0.0f) return false;
	return RaycastModels(origin, origin + Vec3::Up() * maxDistance, hit,
		false, true, nullptr, includeRecoveryFloor);
}

bool ModelCollision::RaycastPillarTop(const Vec3& origin,
	float maxDistance, float topOffset, Physics::RayHit& hit) const
{
	/*
	   プレイヤーの上から下方向にレイを飛ばす。
       Surface::Pillar のモデルだけを調べる。
       当たった面の法線 normal.y を確認する。
       normal.y > 0.5f なら、上向きの面として扱う。
       その面の高さを取得する。
    */

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

	const Vec3 direction = Vec3(moveDirection.x, 0.0f, moveDirection.z).Normalized();//水平の
	bool foundWall = false;
	float bestDirection = 2.0f;
	float bestDistance = detectionRadius;

	const ModelObject* hitPillar = nullptr;//接触中に見つけたモデル用

	for (const auto& entry : models_)
	{
		if (entry.handle < 0 || entry.surface != Surface::Pillar) continue;

		// 表面に触れない柱の内部でも、Spaceを押し直して上昇を再開できる。
		if (entry.boundsReady && feetPosition.y < entry.topY - modelOb.skin &&
			headY > entry.center.y - entry.radii.y)
		{
			const float x = (feetPosition.x - entry.center.x) / entry.radii.x;//足元の位置を楕円の標準化座標に変換する
			const float z = (feetPosition.z - entry.center.z) / entry.radii.z;//これも

			if (x * x + z * z <= 1.0f)//平方根を使わずに、足元が柱の楕円の内部にあるかを判定する
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

			float top = (std::max)({ poly.Position[0].y, poly.Position[1].y, poly.Position[2].y });   //
			float bottom = (std::min)({ poly.Position[0].y, poly.Position[1].y, poly.Position[2].y });//


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

			Vec3 outward = Vec3(normal.x, 0.0f, normal.z).Normalized();//法線の水平値のみで壁は知らん
			const float signedDistance = Vec3::Dot(feetPosition - DxConv::ToVec3(poly.Position[0]), outward);
			//もし正の値だったら、足元が壁の外で、負の値だったら足元が壁の内側にある

			// モデルの面の表裏によらず、壁から移動する物体へ向く法線にそろえる。
			if (signedDistance < 0.0f) outward = -outward;

			const float wallDirection = Vec3::Dot(direction, outward);
			const float distance = std::abs(signedDistance);

			// 角で複数の面に触れた場合も、入力が向かっている面を優先する。
			if (!foundWall || wallDirection < bestDirection - Const::EPS ||
				(std::abs(wallDirection - bestDirection) <= Const::EPS &&
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
		if (entry.handle < 0 || entry.surface == Surface::Stand) continue;
		if (entry.model == fallRecoveryFloor_ && entry.surface == Surface::Floor) continue;

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

// 台に横からぶつかったとき、壁が体を押し返す向きを調べる。
bool ModelCollision::FindStandSide(const Vec3& feetPosition, const ModelOb& modelOb,
	const Vec3& startPosition, const Vec3& direction, Vec3& wallNormal) const
{
	const float radius = modelOb.radius + modelOb.skin;
	const float headY = feetPosition.y + modelOb.height;

	const Vec3 bottom = feetPosition + Vec3::Up() * modelOb.radius;
	const Vec3 top = feetPosition + Vec3::Up() * (modelOb.height - modelOb.radius);

	bool found = false;
	float bestDirection = 0.0f;

	for (const auto& entry : models_)
	{
		if (entry.surface != Surface::Stand || entry.handle < 0 || !entry.boundsReady) continue;

		// 遠くの台や、体より上・下にある台には触れない。
		if (feetPosition.y >= entry.topY - modelOb.skin ||
			headY <= entry.center.y - entry.radii.y + modelOb.skin ||
			std::abs(feetPosition.x - entry.center.x) > entry.radii.x + radius ||
			std::abs(feetPosition.z - entry.center.z) > entry.radii.z + radius) continue;

		// 体を囲む丸い枠が、台の側面に届いているかを見る。
		const auto result = MV1CollCheck_Capsule(entry.handle, -1,
			DxConv::ToVECTOR(bottom), DxConv::ToVECTOR(top), radius);

		for (int i = 0; i < result.HitNum; ++i)
		{
			const auto& triangle = result.Dim[i];
			const Vec3 faceNormal = DxConv::ToVec3(triangle.Normal).Normalized();

			// 台の上に乗るときや、下を通るときは横から引っかけない。
			if (std::abs(faceNormal.y) >= 0.5f) continue;

			const float topY = (std::max)({ triangle.Position[0].y, triangle.Position[1].y, triangle.Position[2].y });
			const float bottomY = (std::min)({ triangle.Position[0].y, triangle.Position[1].y, triangle.Position[2].y });

			if (feetPosition.y >= topY - modelOb.skin || headY <= bottomY + modelOb.skin) continue;

			Vec3 normal = Vec3{ faceNormal.x, 0.0f, faceNormal.z }.Normalized();

			if (normal.LengthSq() == 0.0f) continue;
			//normalの方やったら、台の内側に向いてるので、外側へ
			if (Vec3::Dot(startPosition - DxConv::ToVec3(triangle.Position[0]), normal) < 0.0f)
				normal = -normal;
			//正の値なら壁が外に向いて負の値なら壁が内側に向いてる
			const float wallDirection = Vec3::Dot(direction, normal);

			// 壁から離れるときや、壁に沿って進むときは邪魔しない。
			if (direction.LengthSq() > 0.0f && wallDirection >= -Const::EPS) continue;

			// 角で二つの壁に触れたら、進もうとしているほうの壁を優先する。
			if (!found || wallDirection < bestDirection)
			{
				found = true;
				bestDirection = wallDirection;
				wallNormal = normal;
			}
		}

		MV1CollResultPolyDimTerminate(result);
	}

	return found;
}

// 突進して台をすり抜けないよう、移動先までの道を確かめる
void ModelCollision::SweepStands(const Vec3& startPosition, const Vec3& direction,
	const ModelOb& modelOb, float& travel, Vec3& wallNormal) const
{
	const float checkDistance = modelOb.radius * 0.5f;

	float distance = 0.0f;
	float safeDistance = 0.0f;

	while (true)
	{
		Vec3 normal{};
		const Vec3 position = startPosition + direction * distance;

		if (!FindStandSide(position, modelOb, startPosition, direction, normal))
		{
			if (distance >= travel) return;

			safeDistance = distance;

			// ここまでぶつからなければ、もう少し先を試す。
			distance = (std::min)(distance + checkDistance, travel);

			continue;
		}

		// 台から離れすぎた所で止まらないよう、ぶつかる直前まで近づける。
		float blockedDistance = distance;

		for (int search = 0; search < 12; ++search)
		{
			const float middle = (safeDistance + blockedDistance) * 0.5f;

			if (FindStandSide(startPosition + direction * middle, modelOb, startPosition, direction, normal))
				blockedDistance = middle;
			else
				safeDistance = middle;
		}

		travel = safeDistance;
		wallNormal = normal;

		return;
	}
}

Vec3 ModelCollision::SlideAlongWalls(const Vec3& oldPosition, const Vec3& desiredPosition,
	const ModelOb& modelOb, bool& touched) const
{
	touched = false;

	if (!modelOb.IsValid()) return desiredPosition;

	// 壁に当たっても、ジャンプや落下まで止めない。
	Vec3 position{ oldPosition.x, desiredPosition.y, oldPosition.z };
	Vec3 remaining{ desiredPosition.x - oldPosition.x, 0.0f, desiredPosition.z - oldPosition.z };

	const float rayHeight = modelOb.height * 0.5f;

	for (int attempt = 0; attempt < 4; ++attempt)
	{
		const float length = remaining.Length();

		if (length <= Const::EPS) break;

		const Vec3 direction = remaining / length;
		const Vec3 right{ direction.z, 0.0f, -direction.x };
		const Vec3 center = position + Vec3::Up() * rayHeight;

		float travel = length;
		Vec3 wallNormal{};

		SweepStands(position, direction, modelOb, travel, wallNormal);

		// 体の左右が壁にかすったときも止まれるよう、3か所から確認
		for (int side = -1; side <= 1; ++side)
		{
			const Vec3 start = center + right * (modelOb.radius * side);
			const Vec3 end = start + direction * (length + modelOb.radius + modelOb.skin);

			Physics::RayHit hit{};

			if (!RaycastWall(start, end, hit)) continue;

			const float approaching = -Vec3::Dot(direction, hit.normal);

			if (approaching <= Const::EPS) continue;

			// 壁に体が埋まらんように体の幅の分だけ手前で止める
			const float clearance = modelOb.radius + modelOb.skin +
				(rayHeight - modelOb.radius) * std::abs(hit.normal.y);
			// 壁の法線に沿った距離を、移動方向に沿った距離に変換
			float allowed = (Vec3::Dot(center - hit.point, hit.normal) - clearance) / approaching;

			if (allowed < 0.0f) allowed = 0.0f;

			if (allowed <= travel)
			{
				travel = allowed;
				wallNormal = Vec3{ hit.normal.x, 0.0f, hit.normal.z }.Normalized();
			}
		}

		position += direction * travel;

		if (wallNormal.LengthSq() == 0.0f) break;

		touched = true;
		remaining -= direction * travel;

		// 斜めにぶつかったら、壁沿いにずれる分は残しておく。
		const float inward = Vec3::Dot(remaining, wallNormal);

		if (inward < 0.0f) remaining -= wallNormal * inward;
	}

	return position;
}
