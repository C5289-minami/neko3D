# ImGuiでPlayerの初期値を直接保存する機能

## 何ができるか

ゲーム実行中にImGuiからPlayerの位置・回転・サイズを変更できます。
「ソースに保存」を押すと、`TransformDefaults.h` のC++初期値を直接書き換えます。
Playerの配置を保存するためのJSONや `.txt` ファイルは使いません。

Bossも同じ画面で編集できます。GameとBossTestは別々の初期値を持ちます。

## 操作方法

1. Visual StudioでDebug構成を選び、ゲームを起動します。
2. `Hierarchy` で `Player` を選択します。
3. 必要に応じて「オブジェクトの更新を停止」をオンにします。
4. `Inspector` の数値をドラッグ、または直接入力して編集します。
5. Playerだけを保存する場合は「ソースに保存」を押します。
6. ゲームを終了し、Visual Studioで再ビルドして起動します。

| 項目 | 内容 |
| --- | --- |
| Position | X・Y・Zの位置 |
| Rotation (rad) | X・Y・Zの回転。単位はラジアン |
| Scale | X・Y・Zのモデル倍率。0より大きい値を指定 |

数値を変更すると、実行中のPlayerと描画モデルへすぐに反映されます。
ただし、移動や重力が動いている間は位置が変わるため、固定して調整する場合は更新を停止してください。
Scaleは描画モデルの倍率です。当たり判定用の固定値は変更しません。

## 保存と初期値に戻す操作

| ボタン | 対象と動作 |
| --- | --- |
| Inspectorの「ソースに保存」 | 選択したPlayerまたはBossの初期値を保存 |
| Hierarchyの「シーンをソースに保存」 | 現在のシーンのPlayerとBossをまとめて保存 |
| Inspectorの「初期値に戻す」 | 選択対象を、現在の実行ファイルに含まれる初期値へ戻す |
| Hierarchyの「シーンの初期値に戻す」 | 現在のシーンのPlayerとBossを、そのビルドの初期値へ戻す |

保存はソースファイルへの書き込みです。実行中のプログラムに組み込まれた初期値は、保存ボタンだけでは更新されません。
保存した値を次回の初期値にするには、**保存 → ゲーム終了 → 再ビルド → 起動**の順で操作します。
再ビルド前に「初期値に戻す」を押すと、その実行ファイルをビルドした時点の値に戻ります。ソースは書き換えません。

## 保存先のC++コード

`TransformDefaults.h` の `GamePlayer` が、GameシーンのPlayer初期値です。
今回の競合整理では、保存済みの新しい位置 `{ 0.0f, 36.0f, -521.0f }` を採用しました。

```cpp
// BEGIN_TRANSFORM(game/player)
inline constexpr ObjectTransform GamePlayer{
    { 100.0f, 100.0f, 100.0f }, // サイズ
    { 0.0f, 36.0f, -521.0f },  // 位置
    { 0.0f, 0.0f, 0.0f }      // 回転（ラジアン）
};
// END_TRANSFORM(game/player)
```

この例のコメントは説明用です。保存処理はBEGIN／ENDの目印の間を生成し直します。
目印は保存処理で使用するため残してください。C++側で直接数値を編集して再ビルドすることもできます。

| シーン・対象 | 初期値の名前 | 保存対象のID |
| --- | --- | --- |
| GameのPlayer | GamePlayer | game/player |
| GameのBoss | GameBoss | game/boss |
| BossTestのPlayer | BossTestPlayer | boss_test/player |
| BossTestのBoss | BossTestBoss | boss_test/boss |

## デバッグカメラ中の編集

GameとBossTestのどちらでも、Alt＋Enterでデバッグカメラへ切り替えられます。
切り替え中はオブジェクトの更新を自動停止し、同じHierarchy／Inspectorから編集・保存できます。

Hierarchyで `Camera` を選ぶと、現在使っている通常カメラまたはデバッグカメラの位置を編集できます。
カメラの位置は実行中だけの変更で、Playerの初期値保存には含まれません。
ImGuiで数値を入力したりドラッグしたりしている間は、その入力でデバッグカメラが動かないようにしています。

## 重力チェックとConfigの役割

最新mainにあった「重力を無効化」は、Gameシーンの `Option` ウィンドウに残しました。
これは実行中の重力を切り替える操作で、位置・回転・サイズの保存には含まれません。

`Data/Config/imgui.ini` はImGuiのウィンドウ位置・サイズ・ドッキング配置を保存します。
`Data/Config/toon.json` はトゥーン描画の設定用です。どちらもPlayerの初期値保存とは別の機能です。

## 関係するファイル

| ファイル | 役割 |
| --- | --- |
| [DebugUI.cpp](DebugUI.cpp) | Hierarchy／Inspector、編集欄、保存ボタン |
| [TransformSettings.h](TransformSettings.h) | Vec3を使った位置・回転・サイズと、編集対象の定義 |
| [TransformSettings.cpp](TransformSettings.cpp) | ifstreamでソースを読み、ofstreamで更新内容を書き出す |
| [TransformDefaults.h](TransformDefaults.h) | 保存されたC++初期値 |
| [Player.h](Player.h)・[Player.cpp](Player.cpp) | 編集をPlayerとモデルへ反映し、初期化・リセットで初期値を使用 |
| [GameContext.cpp](GameContext.cpp)・[BossTestScene.cpp](BossTestScene.cpp) | シーンごとの編集対象・カメラ・更新停止を登録 |
| [Debug_camera.cpp](Debug_camera.cpp) | Alt＋Enterと、UI操作中のカメラ入力制御 |
| [Player_camera.cpp](Player_camera.cpp)・[Player_camera.h](Player_camera.h) | 15_Systemから直下へ移した通常カメラ |
| [tests/TransformSettingsTests.cpp](tests/TransformSettingsTests.cpp) | 保存・復元・モデル反映などの動作確認。通常のゲームでは使用しない |

ゲーム内の値は `Vec3` で管理します。ImGuiの `DragFloat3` に渡す間だけ、`EditVector3` 内で `float[3]` に変換します。
保存処理は対象の初期値だけを書き換え、不正な数値や目印の不備がある場合は元のソースを保持します。
書き出したC++がコンパイルできることもテストしています。
