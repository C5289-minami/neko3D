# 壁登り判定・終了処理の参考実装（保留）

保存日: 2026-10-20

この文書は、会話中にAIが追加した壁登り判定・終了処理を、採用を検討するときの参考として保存したものです。
この追加実装はソースから取り外しています。ゲームのビルドでは、このMarkdownのコードは実行されません。

## 保存した案

- 通常の接触判定 `IsHitPillar` と、壁登り専用の `IsNearClimbableWall` を分ける。
- 壁登りでは柱の側面だけを対象にし、足元が側面の上端に達した場合や、体の高さが側面と重ならない場合は登れないとする。
- `Climbing` でSpaceを離した場合、または登れる範囲から出た場合に終了する。
- 終了時は接地・移動入力に応じて `Idle` / `Walk`、空中なら `Jump` に移る。
- 同じ長押しで壁登りを繰り返さないよう、一度Spaceを離してから再開する。
- 上昇量は `PLAYER_CLIMING_SPEED * deltaTime`。登っている間は落下速度の蓄積と床への吸着を止める。

## 判定の考え方

広めのカプセルで柱のポリゴンを候補として取得し、法線と高さで絞り込みます。
カプセルの半径自体は全方向に広がりますが、上面・下面・斜面を除外し、体の本来の高さと側面が重なることを確認します。
X・Z方向だけに変形したカプセルを厳密に計算する実装ではありません。

上端の高さには、その判定で取得した側面の三角形の頂点を使用します。
装飾や段差の多い柱では、実モデルでの調整・確認が必要です。
自動的に柱の上へ乗り移る動作（よじ登りの乗り越え）は含みません。

## 対象ファイル

| ファイル | 保存内容 |
| --- | --- |
| `Stage.h` / `Stage.cpp` | 壁登り専用の側面・高さ判定 |
| `Player.h` / `Player.cpp` | 判定結果、長押しの再開制御、重力・床吸着の制御 |
| `PlayerClimbingState.h` / `PlayerClimbingState.cpp` | 終了条件、戻り先、時間に応じた上昇 |

今回より前の `Climbing` ステート、Space長押し入力、ユーザー作成の範囲判定と移動処理は残しています。
通常の柱との接触判定・移動補正、および下側の中心を10上げる編集も残しています。

## 保存時の確認

- Debug / x64 のビルド成功。検証環境のデバッグ情報DLL不整合を避けるため、検証時だけ `/Z7` と `/DEBUG:NONE` を指定。
- 衝突結果・描画を模擬した32項目の動作確認が成功。
- 実ゲーム上での操作感・実モデルに対する見た目は未確認。

## 追加実装の差分

以下は、今回の追加実装だけを再適用するための保存用差分です。
ユーザーが独立して変更した `capsuleStart` の `+10.0f` は変更対象から除外しています。
再利用するときは現在のソースとの差を確認し、通常の衝突判定やユーザー編集を上書きしないようにしてください。

<details>
<summary>参考実装のコードを表示</summary>

```diff
diff --git a/Player.cpp b/Player.cpp
index c83dc86..dfd105b 100644
--- a/Player.cpp
+++ b/Player.cpp
@@ -39,6 +39,8 @@ void Player::Reset()
 
 	health_ = MaxHealth;
 	wasTouchingPillar_ = false;
+	canClimb_ = false;
+	climbInputConsumed_ = false;
 	model_.collisionHighlighted = false;
 	ultimateGauge_ = 0.0f;
 	// 初期位置・回転・拡大率は既定値を設定した後にJSONから読み込む
@@ -65,7 +67,12 @@ void Player::Update(float deltaTime)
 {
 	input_.Update();
 
-
+	if (!input_.jumpPressing) climbInputConsumed_ = false;
+	if (canClimb_ && input_.jumpPressing && !climbInputConsumed_ && !IsDead() && !IsDamaging())
+	{
+		climbInputConsumed_ = true;
+		stateMachine_.ChangeState(PlayerStateType::Climbing);
+	}
 
 	stateMachine_.Tick(deltaTime);
 
@@ -73,7 +80,12 @@ void Player::Update(float deltaTime)
 	PlayerDiagonalMove(deltaTime, position_, movementCenter_);
 	// 横移動は PlayerDiagonalMove、上下移動は velocity_.y にまとめる。
 
-	if (gravityEnabled_)
+	if (GetCurrentState() == PlayerStateType::Climbing)
+	{
+		// 壁登り中はステート側で高さを更新し、落下速度をためない。
+		StopVerticalMove();
+	}
+	else if (gravityEnabled_)
 	{
 		velocity_.y -= Const::GRAVITY * deltaTime;
 	}
@@ -91,13 +103,15 @@ void Player::Update(float deltaTime, const Stage& stage, const Vec3& movementCen
 	const Vec3 oldPosition = GetPosition();
 	const float previousY = oldPosition.y;
 
+	canClimb_ = stage.IsNearClimbableWall(GetPosition(),
+		Const::PLAYER_HEIGHT, Const::PLAYER_RADIUS, climbMargin_);
 	Update(deltaTime);
 
 	// ステージの柱に沿ってスライドさせる
     position_ = stage.SlideAlongPillars(oldPosition, position_, touchingPillar);
 
 	// 落下中のみ床面に合わせ、このフレームの落下距離もレイの長さに含める。
-	if (velocity_.y <= 0.0f)
+	if (velocity_.y <= 0.0f && GetCurrentState() != PlayerStateType::Climbing)
 	{
 		Physics::RayHit hit{};
 		constexpr float groundSnapDistance = 30.0f;
@@ -138,17 +152,8 @@ void Player::Update(float deltaTime, const Stage& stage, const Vec3& movementCen
     }
     wasTouchingPillar_ = touchingPillar;
 
-		//Climingのためのもの
-	CanCliming = stage.IsHitPillar(capsuleStart, capsuleEnd, Const::PLAYER_RADIUS + excessSize);
-	/*	if (IsCanJump&&input_.jumpPressing)
-		{
-			position_.y += Const::PLAYER_CLIMING_SPEED;
-		}*/
-	if (CanCliming && input_.jumpPressing)
-	{
-		stateMachine_.ChangeState(PlayerStateType::Climbing);
-	}
-		
+	canClimb_ = stage.IsNearClimbableWall(GetPosition(),
+		Const::PLAYER_HEIGHT, Const::PLAYER_RADIUS, climbMargin_);
 
 	Sound3D::SetListener(
 		GetPosition(),
diff --git a/Player.h b/Player.h
index d2934fd..44993b7 100644
--- a/Player.h
+++ b/Player.h
@@ -40,6 +40,8 @@ public:
 	// 移動
 	void SetMoveDirection(const Vec3& moveDir);
 	void StopMove();
+    void StopVerticalMove() { velocity_.y = 0.0f; }
+    bool CanClimb() const { return canClimb_; }
 
 	// 回転
 	void Turn(float direction, float speed, float deltaTime);
@@ -111,6 +113,7 @@ private:
     bool gravityEnabled_{ true };
 
 
-    bool CanCliming{false};
-    float excessSize{20.f};
+    bool canClimb_{ false };
+    bool climbInputConsumed_{ false };
+    float climbMargin_{ 20.0f };
 };
diff --git a/PlayerClimbingState.cpp b/PlayerClimbingState.cpp
index 872326f..1f1cc2c 100644
--- a/PlayerClimbingState.cpp
+++ b/PlayerClimbingState.cpp
@@ -10,28 +10,34 @@ PlayerClimbingState::PlayerClimbingState(PlayerStateMachine& stateMachine, Playe
 void PlayerClimbingState::Enter()
 {
     player_.StopMove();
+    player_.StopVerticalMove();
+    player_.SetGround(false);
     // 壁登り用のアニメーションを用意するまでは、既存の待機アニメーションを使う。
     player_.SetAnimation(PlayerAnimType::Idle, true);
     DxPlus::Debug::SetFormatString(L"PlayerClimbingState::Enter");
 }
 
-void PlayerClimbingState::Tick(float)
+void PlayerClimbingState::Tick(float deltaTime)
 {
     const PlayerInput& input = player_.GetInput();
-	Vec3 playerPos = player_.GetPosition();
-
-    // ジャンプボタンを押している間、変更した座標をプレイヤーに反映する。
-    DxPlus::Debug::SetFormatString(L"state:climbing");
-    if(input.jumpPressing)
+    if (!input.jumpPressing || !player_.CanClimb())
     {
-		playerPos.y += 100.f/2.0f; // 上昇
-        player_.SetPosition(playerPos);
+        if (player_.IsGrounded() || !player_.IsGravityEnabled())
+            stateMachine_.ChangeState(input.isMoving ? PlayerStateType::Walk : PlayerStateType::Idle);
+        else
+            stateMachine_.ChangeState(PlayerStateType::Jump);
+        return;
     }
 
+    Vec3 playerPos = player_.GetPosition();
+    playerPos.y += Const::PLAYER_CLIMING_SPEED * deltaTime;
+    player_.SetPosition(playerPos);
+    DxPlus::Debug::SetFormatString(L"state:climbing");
 }
 
 void PlayerClimbingState::Exit()
 {
     player_.StopMove();
+    player_.StopVerticalMove();
     DxPlus::Debug::SetFormatString(L"PlayerClimbingState::Exit");
 }
diff --git a/PlayerClimbingState.h b/PlayerClimbingState.h
index 404f69d..e5b7cf9 100644
--- a/PlayerClimbingState.h
+++ b/PlayerClimbingState.h
@@ -1,7 +1,7 @@
 ﻿#pragma once
 #include "PlayerBaseState.h"
 
-// 壁登り用のステート。遷移条件と移動処理は後で接続する。
+// 側面を登り、長押しの終了や範囲外への移動で通常の行動に戻る。
 class PlayerClimbingState : public PlayerBaseState
 {
 public:
diff --git a/Stage.cpp b/Stage.cpp
index 4cf2f03..bbbc362 100644
--- a/Stage.cpp
+++ b/Stage.cpp
@@ -144,6 +144,45 @@ bool Stage::IsHitPillar(const Vec3& capsuleStart, const Vec3& capsuleEnd, float
     return false;
 }
 
+bool Stage::IsNearClimbableWall(const Vec3& feetPosition, float height, float radius, float horizontalMargin) const
+{
+    if (radius <= 0.0f || height < radius * 2.0f || horizontalMargin < 0.0f) return false;
+
+    const Vec3 capsuleStart = feetPosition + Vec3::Up() * radius;
+    const Vec3 capsuleEnd = feetPosition + Vec3::Up() * (height - radius);
+    const float headY = feetPosition.y + height;
+
+    for (int handle : pillarCollisionHandles_)
+    {
+        if (handle < 0) continue;
+
+        // 広いカプセルで候補を集め、側面と本来の高さの範囲だけを残す。
+        const auto result = MV1CollCheck_Capsule(handle, -1,
+            DxConv::ToVECTOR(capsuleStart), DxConv::ToVECTOR(capsuleEnd), radius + horizontalMargin);
+        bool canClimb = false;
+        for (int i = 0; i < result.HitNum; ++i)
+        {
+            const auto& poly = result.Dim[i];
+            const Vec3 normal = DxConv::ToVec3(poly.Normal).Normalized();
+            if (normal.LengthSq() == 0.0f || std::abs(normal.y) > 0.01f) continue;
+
+            const float top = (std::max)(poly.Position[0].y,
+                (std::max)(poly.Position[1].y, poly.Position[2].y));
+            const float bottom = (std::min)(poly.Position[0].y,
+                (std::min)(poly.Position[1].y, poly.Position[2].y));
+
+            // 上に登り切った側面や、頭より上にしかない側面では登れない。
+            if (feetPosition.y >= top - Const::PLAYER_SKIN || headY <= bottom) continue;
+
+            canClimb = true;
+            break;
+        }
+        MV1CollResultPolyDimTerminate(result);
+        if (canClimb) return true;
+    }
+    return false;
+}
+
 Vec3 Stage::SlideAlongPillars(const Vec3& oldPosition, const Vec3& desiredPosition, bool& touched) const
 {
    
diff --git a/Stage.h b/Stage.h
index 3456173..f0230bb 100644
--- a/Stage.h
+++ b/Stage.h
@@ -31,6 +31,8 @@ public:
 
     // プレイヤーのカプセルと柱L・M・Sのメッシュを判定する。
     bool IsHitPillar(const Vec3& capsuleStart, const Vec3& capsuleEnd, float radius) const;
+    // 側面への近さと、余裕を加えない体の高さで壁登りできるか調べる。
+    bool IsNearClimbableWall(const Vec3& feetPosition, float height, float radius, float horizontalMargin) const;
     Vec3 SlideAlongPillars(const Vec3& oldPosition, const Vec3& desiredPosition, bool& touched) const;
 
     void DrawDebugUI();
```

</details>
