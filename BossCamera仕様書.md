# ボス戦カメラ仕様書

## 1. 概要

本書は、現在の `Player_camera` と `GameContext` に実装されているボス戦カメラの仕様をまとめる。

このカメラは、ボスからプレイヤーへ向かう水平方向を基準に、プレイヤーの外側へカメラを置く。注視点はボスとプレイヤーの中間に置き、プレイヤーとボスの両方を画面に入れやすくする。プレイヤーやボスの移動に合わせて、カメラ位置と注視点を毎フレーム更新する。

## 2. 対象ファイル

| ファイル | 役割 |
| --- | --- |
| `15_System/Player_camera.h` | カメラ設定値、位置・注視点・上方向の保持、公開関数 |
| `15_System/Player_camera.cpp` | ボスとプレイヤーの座標から目標カメラ位置を計算し、追従させる |
| `GameContext.h` | 通常カメラ、プレイヤー、ボスの所有 |
| `GameContext.cpp` | 初期化・更新・描画時の接続 |

## 3. 座標と向き

ワールド座標は `Vec3` を使う。`x` と `z` が地面上の位置、`y` が高さを表す。

カメラ更新関数の引数順は次の通り。

```cpp
playerCamera.Update(player.GetPosition(), boss.GetPosition(), deltaTime);
```

1番目がプレイヤー位置、2番目がボス位置、3番目がフレーム経過時間（秒）。両方とも `Vec3` 型のため引数を逆にしてもコンパイルエラーにならないが、計算上の意味が変わるので順番を守る。

プレイヤーとボスの座標はそれぞれのオブジェクトが管理する。カメラは引数で座標を受け取り、カメラ自身の状態として `cameraPosition`（カメラ位置）、`lookAtPosition`（注視点）、`upDirection`（上方向）を保持する。

## 4. 1フレームごとの計算

`Player_camera::Update` は次の順で処理する。

### 4.1 ボスからプレイヤーへの水平向きを求める

```cpp
Vec3 directionFromBossToPlayer = playerPosition - bossPosition;
directionFromBossToPlayer.y = 0.0f;
```

まず「ボスからプレイヤーへ向かうベクトル」を作り、`y` を 0 にする。高さの差は無視し、地面上での向きだけを使う。

このベクトルの長さがほぼ 0 の場合、正規化できる方向がない。その場合は `{ 0, 0, 1 }` を仮の向きとして使う。それ以外は `Normalized()` で長さを 1 にそろえる。

### 4.2 カメラの目標位置を求める

```cpp
targetCameraPosition = playerPosition
          + directionFromBossToPlayer * cameraDistance
          + (0, cameraHeight, 0)
```

`directionFromBossToPlayer` はボスからプレイヤーへ向くので、これをプレイヤー位置に足すとプレイヤーよりさらに外側になる。そこから `cameraDistance` だけ離し、`cameraHeight` だけ上げた位置がカメラの目標位置 `targetCameraPosition`。

### 4.3 注視点を求める

```cpp
targetLookAtPosition = (bossPosition + playerPosition) * 0.5
             + (0, targetHeight, 0)
```

2つの座標を足して 2 で割ることで中間地点を求め、その地点を `targetHeight` だけ上げる。カメラはこの点を見る。

### 4.4 現在位置から目標へ近づける

```cpp
movementRate = followSpeed * deltaTime;
movementAmount = min(movementRate, 1.0f);
cameraPosition += (targetCameraPosition - cameraPosition) * movementAmount;
lookAtPosition += (targetLookAtPosition - lookAtPosition) * movementAmount;
```

目標位置へ一度に移動せず、現在位置と目標位置の差の `amount` 分だけ移動する。`amount` が `1.0f` 以上になる場合は `1.0f` に抑え、目標を通り越さないようにする。

## 5. 調整値

設定値は `Player_camera.h` の `PlayerCameraSettings` にまとめてあり、初期値は次の通り。

| 設定名 | 初期値 | 役割 |
| --- | ---: | --- |
| `cameraDistance` | `900.0f` | プレイヤーから外側へ離す距離 |
| `cameraHeight` | `420.0f` | プレイヤー位置からカメラを上げる量 |
| `targetHeight` | `110.0f` | ボスとプレイヤーの中間点を上げる量 |
| `followSpeed` | `5.0f` | 1秒あたりの追従の強さ。大きいほど速く近づく |

距離や高さの単位はゲーム内ワールド座標の単位。モデルやステージの大きさに合わせて調整する。

設定値は `GameContext` から次のように変更できる。

```cpp
gameContext.GetPlayerCamera().GetSettings().cameraDistance = 1000.0f;
```

設定を固定値のまま使う場合は、`Player_camera` 内にある `settings` の初期値を変更すればよい。

## 6. 更新・描画の流れ

通常の `GameContext::Update` では、次の順に処理する。

1. プレイヤーを更新する。
2. ボスを更新する。
3. 更新後のプレイヤー位置とボス位置をカメラへ渡す。
4. カメラが `cameraPosition` と `lookAtPosition` を更新する。

`GameContext::Draw` では `GetCameraPosition()`、`GetCameraTarget()`、`GetCameraUp()` を使って、次のDxLib関数でカメラを適用する。

```cpp
DxLib::SetCameraPositionAndTargetAndUpVec(cameraPosition, lookAtPosition, upDirection);
```

通常時は `Player_camera` の値を使う。Debugビルドでデバッグカメラ操作中の場合は、`GameContext` の取得関数が `Debug_camera` の値を返す。

## 7. 初期化とリセット

`Player_camera::Reset` は次の値へ戻す。

- `cameraPosition`: `(400, 400, 400)`
- `lookAtPosition`: `(0, 0, 0)`
- `upDirection`: `(0, 1, 0)`

通常更新が始まると、`Update` が計算した目標位置へ追従を始める。`Reset` は `PlayerCameraSettings` の値を変更しない。

## 8. 現状の範囲と注意点

- カメラの回り込み方向はプレイヤーの向きや移動方向ではなく、ボスとプレイヤーの相対位置から決める。
- 注視点は2者の中間地点を使う。画面端の余白、画角、ボスとプレイヤーの距離に応じたズームは計算していない。
- 地形との衝突判定はないため、カメラが壁やステージにめり込む可能性がある。
- `followSpeed` は線形補間の係数に使われる。初期値 `5.0f` のとき、`deltaTime` が約 `0.2` 秒以上なら1フレームで目標へ到達する。通常の短いフレームでは、そのフレーム時間に応じた割合だけ近づく。
- 現在の `Boss::Reset` はボス座標の `y` を `-1000.0f` に設定している。カメラはその座標をそのまま注視点計算に使うため、実際のボス配置が別の高さなら、ボスの配置とカメラの見え方を合わせて確認する。
- `deltaTime` は 0 以上を渡す想定。負値を受け取った場合の補正は実装していない。

## 9. 変更時に確認すること

1. `Update` の宣言・定義・呼び出しで、引数順が「プレイヤー、ボス、deltaTime」になっていること。
2. ステージとキャラクターのスケールに対して、距離・高さの初期値が適切であること。
3. プレイヤーがボスの周囲を移動したとき、カメラが外側へ回り込み、両者が見えること。
4. プレイヤーとボスがほぼ同じ XZ 座標になったとき、仮の向きによってカメラ位置が不定にならないこと。
5. Debugカメラ操作中は通常カメラの描画値ではなく、デバッグカメラの値が使われること。
