# neko3D

## 開発環境

Visual Studio 2022 の「C++ によるデスクトップ開発」で `3dgp1.sln` を開き、
`Debug | x64` または `Release | x64` でビルド・実行できます。

DxLib は `DxLib.props` で設定しています。PC 内の
`C:\DxLib_VC\Dxlib`（現在は 3.25a）を優先し、そのフォルダーがない場合は
プロジェクト内の `DxLib` を参照します。ヘッダーとライブラリは同じフォルダーから
読み込むため、次回プロジェクトを開いたときも再設定やダウンロードは不要です。

配置先を変更した場合は `DxLib.props` のパスを変更するか、MSBuild の
`/p:DxLibDir="配置先"` で指定してください。
NuGet パッケージがない場合は、ソリューションの「NuGet パッケージの復元」を実行します。

## ボス戦カメラ仕様

### 概要

通常ゲーム中のカメラは `Player_camera` が管理します。ボスからプレイヤーへの地面上の向きを使い、プレイヤーの外側にカメラを置きます。注視点はプレイヤーとボスの中間付近です。プレイヤーがボスの周りを移動すると、座標から毎フレーム置き場所と注視点を計算し直します。

カメラのYaw角を直接指定する方式ではありません。`eye` に相当するカメラ位置と `target` に相当する注視点を `SetCameraPositionAndTargetAndUpVec` に渡し、DxLibに2点を向くカメラとして設定します。

### 座標からカメラ位置を求める

更新関数の引数順は「プレイヤー位置、ボス位置、経過時間（秒）」です。

```cpp
playerCamera.Update(player.GetPosition(), boss.GetPosition(), deltaTime);
```

まずボスからプレイヤーへ向かうベクトルを作り、Y成分を0にして水平向きだけにします。方向がほぼゼロの場合は仮の向き `(0, 0, 1)` を使い、それ以外は長さ1に正規化します。

```cpp
directionFromBossToPlayer = playerPosition - bossPosition;
directionFromBossToPlayer.y = 0.0f;
directionFromBossToPlayer = directionFromBossToPlayer.Normalized();
```

カメラ位置の目標は、プレイヤー位置からその方向へ `cameraDistance` 進み、さらに `cameraHeight` 上げた位置です。

```cpp
targetCameraPosition = playerPosition
    + directionFromBossToPlayer * cameraDistance
    + Vec3(0.0f, cameraHeight, 0.0f);
```

注視点の目標は、ボスとプレイヤーの中間点を `targetHeight` だけ上げた位置です。

```cpp
targetLookAtPosition = (bossPosition + playerPosition) * 0.5f
    + Vec3(0.0f, targetHeight, 0.0f);
```

### カメラの追従

カメラ位置と注視点は、目標へ一度に移動させず、現在位置から少しずつ近づけます。

```cpp
movementAmount = min(followSpeed * deltaTime, 1.0f);
cameraPosition += (targetCameraPosition - cameraPosition) * movementAmount;
lookAtPosition += (targetLookAtPosition - lookAtPosition) * movementAmount;
```

`followSpeed` の初期値は `5.0f` です。値を上げると追従が速くなります。60 FPS なら1フレームごとに残りの差の約8.3%を詰めます。`movementAmount` は最大 `1.0f` なので、目標位置を通り越しません。

### カメラ設定値

設定値は `15_System/Player_camera.h` の `PlayerCameraSettings` にあります。実行中は `GetPlayerCamera().GetSettings()` から変更できます。

| 設定 | 初期値 | 内容 |
| --- | ---: | --- |
| `cameraDistance` | `900.0f` | プレイヤーからカメラを外側へ離す距離 |
| `cameraHeight` | `420.0f` | プレイヤー位置からカメラを上げる量 |
| `targetHeight` | `110.0f` | ボスとプレイヤーの中間点を上げる量 |
| `followSpeed` | `5.0f` | カメラ位置・注視点が目標へ近づく速さ |

距離と高さの単位はゲーム内ワールド座標です。ステージやモデルの大きさに合わせて調整します。

### 初期化と更新の流れ

- `GameContext::Init()` と `GameContext::Reset()` でプレイヤー、ボス、カメラを初期化します。
- 通常更新ではプレイヤーとボスを更新してから、2者の位置をカメラへ渡します。
- 描画時はカメラ位置、注視点、上方向をDxLibに設定してからステージとキャラクターを描画します。
- 通常カメラの初期値は位置 `(400, 400, 400)`、注視点 `(0, 0, 0)`、上方向 `(0, 1, 0)` です。追従更新後は計算した目標へ近づきます。

### デバッグカメラ

Debugビルドでは左 Alt + Enter で、通常カメラと自由操作カメラを切り替えられます。これはシーン確認用の機能です。プレイヤー座標のデバッグ調整には切り替え不要です。

| 操作 | 動作 |
| --- | --- |
| 左 Alt + Enter | 自由操作カメラの開始・終了 |
| マウス移動 | 視点の回転 |
| W / S | 視線方向へ前進・後退 |
| A / D | 左右移動 |
| Q / E | Y軸方向へ下降・上昇 |
| 左 Shift | 移動速度を3倍にする |
| R | デバッグ開始時の視点へ戻す |
| F | プレイヤーの現在位置へ注目する |

デバッグカメラ操作中はプレイヤーと通常カメラの更新を停止します。Fキーは押した時点の位置へ向く操作で、継続追従ではありません。デバッグ移動速度の初期値は `500.0f` です。

### プレイヤー位置のデバッグ調整

Debugビルドの `Player debug` ウィンドウで、`Position (X, Y, Z)` を左クリックしたまま左右にドラッグするとプレイヤーの座標を変更できます。カメラを切り替えたり移動したりする必要はありません。ドラッグ速度は `1.0` です。

開始位置とリセット位置はどちらも `(X=0, Y=300, Z=2300)` です。位置欄はワールド座標を直接設定するため、Yを変更すると高さも変わります。通常のプレイヤー移動や重力が動作中なら、その後のゲーム更新で座標が変化することがあります。

### 現在の制約

- ボスとプレイヤーが画面内に必ず収まるような画角・距離調整はしていません。距離は設定値で固定です。
- カメラとステージの衝突判定はありません。
- 注視点は2者の中間点を使う単純な計算です。
- `Boss::Reset()` の現在のボス位置は `(0, -1000, 0)` です。カメラもこの座標を注視点計算に使うので、実際にボスを配置する高さと合わせて確認してください。
