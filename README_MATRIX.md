# MATRIXの読み方と、このゲームでの使い方

このゲームでは、`MATRIX`を使って**表示するモデルと当たり判定用モデルの位置・向き・大きさをそろえている**。

まずは、次の3行を押さえると処理を追いやすい。

```cpp
entry.model->ApplyTransform();                  // モデルの配置をDxLibへ反映する
const MATRIX world = MV1GetMatrix(source);      // その配置を行列として取り出す
MV1SetMatrix(entry.handle, world);              // 判定用モデルにも同じ配置を渡す
```

実際のコードでは、初回の準備と、配置が変わったときの更新に分けている。
そのため、この3行は常に続けて実行されるわけではない。
実装は[ModelCollision.cpp](ModelCollision.cpp)の`ModelCollision::Refresh()`にある。

## 1. MATRIXとは

`MATRIX`は、DxLibが用意している「行列」の構造体。
行列とは、数字を縦横に並べたもの。`MATRIX`には`float`の数字が4行×4列、合計16個入っている。

中身の形を簡略化すると、こうなっている。

```cpp
struct MATRIX
{
    float m[4][4];
};
```

```text
           列0       列1       列2       列3
行0    m[0][0]   m[0][1]   m[0][2]   m[0][3]
行1    m[1][0]   m[1][1]   m[1][2]   m[1][3]
行2    m[2][0]   m[2][1]   m[2][2]   m[2][3]
行3    m[3][0]   m[3][1]   m[3][2]   m[3][3]
```

`m[行][列]`の順番で数字を取り出す。配列なので、番号は`0`から`3`まで。
例えば、`world.m[2][1]`は「行2・列1にある数字」を表す。

今回のモデル用の行列は、モデル内の座標を、ゲーム空間の座標へ変換するために使う。
位置・回転・拡大率が、その変換を行う数字としてまとまっている。

## 2. モデル内の座標と、ゲーム空間の座標

例えば、柱モデルのある頂点が、モデル内で`(10, 0, 0)`にあるとする。
柱を2倍にして、ゲーム空間の`(100, 0, 200)`へ置く。回転はさせない。

この頂点の位置は、次のように変わる。

```text
モデル内の頂点       (10, 0,   0)
        ↓ 2倍にする
                     (20, 0,   0)
        ↓ (100, 0, 200)だけ移動する
ゲーム空間の頂点    (120, 0, 200)
```

この「2倍にして、指定した位置へ移す」という変換を、行列で表せる。
モデルの回転があれば、その向きの変換も含められる。

DxLibの、このようなモデル変換の行列では、回転なし・拡大率2倍・移動量`(100, 0, 200)`は次の形になる。
この例では、モデルの中心補正や親の変換は省いている。

```text
  2    0    0    0
  0    2    0    0
  0    0    2    0
100    0  200    1
```

この場合、最後の行の`m[3][0]`・`m[3][1]`・`m[3][2]`が移動量のX・Y・Zになる。
回転が入ると、左上の3行×3列には回転と拡大率が組み合わさった数字が入る。
その場合、斜めに並んだ数字だけを読んで「これが拡大率」とは判断できない。

4行×4列にすることで、3Dの回転・拡大縮小と、平行移動をひとつの行列で扱える。
今回の使い方では、16個の値を自分で作らず、DxLibから完成した行列を取得している。

## 3. なぜ当たり判定に必要なのか

`ModelCollision`は、登録したモデルごとに、判定専用の複製モデルを持っている。

例えば、同じ柱モデルを次のように2本置くことができる。

| 配置した柱 | 位置 | 拡大率 |
| --- | --- | --- |
| 柱A | `(100, 0, 0)` | 1倍 |
| 柱B | `(500, 0, 0)` | 2倍 |

2本とも同じモデルデータを使うが、判定用モデルにはそれぞれの配置が必要になる。
そこで、柱Aの変換行列を柱Aの判定用モデルへ、柱Bの変換行列を柱Bの判定用モデルへ渡している。

`MATRIX`を渡す処理で、モデルの頂点データや`Surface`の種類を変更するわけではない。
モデルをゲーム空間のどこに、どの向き・大きさで置くかをそろえる。

## 4. ApplyTransform()で、何を準備しているのか

`ModelObject`の位置や拡大率を変更すると、その値はまず`ModelObject`側に保存される。
`ApplyTransform()`は、その値をDxLibのモデルへ反映する。
実装は[DrawableObject.cpp](DrawableObject.cpp)にある。

```cpp
const Vec3 modelPosition = position + masterPos - RotateXYZ(center, rotation);

MV1SetPosition(handle, DxConv::ToVECTOR(modelPosition));
MV1SetScale(handle, DxConv::ToVECTOR(scale));
MV1SetRotationXYZ(handle, DxConv::ToVECTOR(rotation));
```

- `modelPosition`：追加の移動量と中心の補正を含めた位置。
- `scale`：拡大率。
- `rotation`：回転。

親が設定されているモデルでは、さらに`WithParentTransform()`で親の位置・回転を合成して反映する。
例えば、ステージの子になっている柱は、柱自身の配置に加えてステージの位置・回転も反映される。

この準備をしてから`MV1GetMatrix()`を呼ぶことで、現在の配置に対応した行列を取得できる。
`Draw()`の中でも`ApplyTransform()`を使っているため、描画前にも同じ配置を判定側へ渡せる。

## 5. world・source・entry.handleは何か

`ModelCollision::Refresh()`に出てくる名前は、次の意味。

| 名前 | 中身 |
| --- | --- |
| `entry.model` | 登録された`ModelObject`へのポインター。配置の設定を参照する |
| `source` | ResourceManagerから取得した、元モデルのハンドル |
| `entry.handle` | その登録モデル専用に作った、判定用モデルのハンドル |
| `world` | 元モデルへ現在の配置を反映した後、その変換行列をコピーした値 |
| `previous` | 更新する前の、判定用モデルが持っている変換行列のコピー |

ハンドルは、DxLibがモデルを識別するための`int`の番号。
`source`や`entry.handle`は番号であり、行列そのものは`world`や`previous`に入っている。

```cpp
const MATRIX world = MV1GetMatrix(source);
```

これは「`source`番のモデルから変換行列を取り出し、`world`という変数へ入れる」という意味。
`const`は、この変数に入れた行列を、ここから書き換えないことを表す。

```cpp
MV1SetMatrix(entry.handle, world);
```

これは「判定用モデルへ、`world`の変換行列を設定する」という意味。
名前が`world`なのは、ゲーム空間の配置を表す行列として扱っているため。

## 6. 初めて判定用モデルを作るとき

現在のコードは、次の順番で準備している。ここでは、失敗時の処理を省略している。

```cpp
entry.model->ApplyTransform();
const MATRIX world = MV1GetMatrix(source);

entry.handle = MV1DuplicateModel(source);      // 判定用モデルを複製する
MV1SetMatrix(entry.handle, world);            // 現在の配置を渡す
MV1SetupCollInfo(entry.handle, -1, 8, 8, 8);  // その配置で判定用データを準備する
```

順番は「配置を反映する → 行列を取得する → 判定用モデルへ設定する → 判定データを準備する」。

## 7. row・columnで何を比較しているのか

一度準備した後は、判定用モデルが持つ配置と、今回反映した配置を比較している。

```cpp
const MATRIX previous = MV1GetMatrix(entry.handle);
bool changed = false;

for (int row = 0; row < 4; ++row)
    for (int column = 0; column < 4; ++column)
        changed |= previous.m[row][column] != world.m[row][column];
```

`row`は行番号、`column`は列番号。二重のループで、16個の数字をひとつずつ比較する。
どれかひとつでも違えば、`changed`が`true`になる。

この行は少し短く書いてある。

```cpp
changed |= previous.m[row][column] != world.m[row][column];
```

今回の`bool`変数に対する処理は、次のように読める。

```cpp
if (previous.m[row][column] != world.m[row][column])
{
    changed = true;
}
```

一度`true`になったら、残りの数字が同じでも`false`には戻らない。
位置だけを比較すると回転や拡大率の変化を見落とすため、16個すべてを比較している。
現在は完全一致で比較しているので、とても小さな数値の違いでも更新の対象になる。

## 8. 変わっていたら何をするのか

```cpp
if (changed)
{
    MV1SetMatrix(entry.handle, world);
    MV1RefreshCollInfo(entry.handle, -1);
}
```

まず判定用モデルの変換行列を更新し、その後で当たり判定用のデータも更新する。
この例でも、失敗時の処理は省略している。

| 処理 | 役割 |
| --- | --- |
| `MV1SetMatrix` | 判定用モデルの位置・回転・拡大率を更新する |
| `MV1RefreshCollInfo` | 更新後の配置に合わせて、当たり判定用のデータを更新する |

配置が変わらなければ、この再計算を省く。

## 9. サイズ取得にも同じ行列を使う

`ModelCollision::GetSize()`にも、次の行がある。

```cpp
MV1SetMatrix(sizeHandle, MV1GetMatrix(entry.handle));
```

分けて読むと、こうなる。

```cpp
const MATRIX world = MV1GetMatrix(entry.handle);
MV1SetMatrix(sizeHandle, world);
```

サイズ取得専用の複製モデルにも、判定用モデルと同じ変換を設定している。
その後、変換後の頂点の最大座標から最小座標を引いて、幅・高さ・奥行きを求める。
サイズを知るときは先に`Refresh()`を呼び、現在の配置を判定用モデルへ反映しておく。

`MATRIX`だけからモデルの実寸がわかるわけではない。
例えば拡大率が2倍でも、元のモデルが幅10なのか幅100なのかで、最終的な幅は変わる。
実寸の取得にはモデルの頂点も使っている。

## 10. ほかの名前との違い

| 名前 | このプロジェクトでの役割 |
| --- | --- |
| `ModelObject` | モデルのキー・位置・回転・拡大率などの設定を持つ |
| `MATRIX` | 設定を反映したモデルの座標変換を、4行×4列の数字で表す |
| `ModelOb` | 動く側の判定用カプセルの半径・高さ・余白を持つ |
| `Surface` | 登録したモデルを床・壁・登れる壁のどれとして使うかを指定する |

`DrawableObject.cpp`内の`RotationMatrix`は、親子の回転を合成するための3行×3列の配列。
今回の`MATRIX`とは別の型だが、どちらも回転などを数字で扱うために使っている。

## コードを追う順番

1. [Player.cpp](Player.cpp)の`Player::Update()`で、`collision.Refresh()`を呼ぶ。
2. [ModelCollision.cpp](ModelCollision.cpp)の`Refresh()`で、モデルごとの配置を反映する。
3. [DrawableObject.cpp](DrawableObject.cpp)の`ApplyTransform()`で、DxLibへ位置・回転・拡大率を設定する。
4. `Refresh()`へ戻り、行列を取得して判定用モデルと比較し、必要なら更新する。
5. プレイヤーの更新へ戻り、用意できた判定用モデルで壁・滑り・着地を調べる。

`MATRIX`の型定義と各APIの宣言は、使用中のDxLibの`DxLib.h`で確認できる。
プロジェクトに同梱している定義は[DxLib/DxLib.h](DxLib/DxLib.h)にあり、実際の参照先は[DxLib.props](DxLib.props)で選んでいる。
