Texture2D g_DepthTexture : register(t0);

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 ViewPosition : TEXCOORD2;
};


// ================================
// 法線取得
// ================================

float3 LoadNormal(
    Texture2D textureData,
    uint2 pixel
)
{
    float3 normal =
        textureData.Load(
            int3(pixel, 0)
        ).rgb;

    // 0～1 → -1～1
    normal =
        normal * 2.0f - 1.0f;

    return normalize(normal);
}


// ================================
// メイン
// ================================

float4 main(PS_INPUT input) : SV_TARGET
{
    uint width;
    uint height;

    g_DepthTexture.GetDimensions(
        width,
        height
    );

    uint2 pixel =
        uint2(
            input.Position.x,
            input.Position.y
        );

    pixel.x =
        min(pixel.x, width - 1);

    pixel.y =
        min(pixel.y, height - 1);


    // ----------------------------
    // 中央
    // ----------------------------

    float3 centerNormal =
        LoadNormal(
            g_DepthTexture,
            pixel
        );


    // ----------------------------
    // 左
    // ----------------------------

    uint2 leftPixel =
        pixel;

    leftPixel.x =
        (pixel.x > 0)
        ? pixel.x - 1
        : 0;

    float3 leftNormal =
        LoadNormal(
            g_DepthTexture,
            leftPixel
        );


    // ----------------------------
    // 右
    // ----------------------------

    uint2 rightPixel =
        pixel;

    rightPixel.x =
        min(
            pixel.x + 1,
            width - 1
        );

    float3 rightNormal =
        LoadNormal(
            g_DepthTexture,
            rightPixel
        );


    // ----------------------------
    // 上
    // ----------------------------

    uint2 upPixel =
        pixel;

    upPixel.y =
        (pixel.y > 0)
        ? pixel.y - 1
        : 0;

    float3 upNormal =
        LoadNormal(
            g_DepthTexture,
            upPixel
        );


    // ----------------------------
    // 下
    // ----------------------------

    uint2 downPixel =
        pixel;

    downPixel.y =
        min(
            pixel.y + 1,
            height - 1
        );

    float3 downNormal =
        LoadNormal(
            g_DepthTexture,
            downPixel
        );


    // ----------------------------
    // 法線差
    // ----------------------------

    float leftDifference =
        length(
            centerNormal -
            leftNormal
        );

    float rightDifference =
        length(
            centerNormal -
            rightNormal
        );

    float upDifference =
        length(
            centerNormal -
            upNormal
        );

    float downDifference =
        length(
            centerNormal -
            downNormal
        );


    // ----------------------------
    // 最大値
    // ----------------------------

    float edge =
        max(
            max(
                leftDifference,
                rightDifference
            ),
            max(
                upDifference,
                downDifference
            )
        );


    // ----------------------------
    // 見やすくする
    // ----------------------------

    edge =
        saturate(
            edge * 3.0f
        );


    // ----------------------------
    // 赤で表示
    // ----------------------------

    return float4(
        edge,
        0.0f,
        0.0f,
        1.0f
    );
}