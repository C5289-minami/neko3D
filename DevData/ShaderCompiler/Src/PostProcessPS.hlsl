Texture2D g_SceneTexture : register(t0);
Texture2D g_DepthTexture : register(t1);

SamplerState g_Sampler : register(s0);

cbuffer OutlineSettings : register(b6)
{
    float4 Outline;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
};


// ================================
// 深度
// ================================

float LoadDepth(uint2 pixel)
{
    return g_DepthTexture.Load(
        int3(pixel, 0)
    ).a;
}


// ================================
// 法線
// ================================

float3 LoadNormal(uint2 pixel)
{
    float3 normal =
        g_DepthTexture.Load(
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
    // ================================
    // テクスチャサイズ
    // ================================

    uint width;
    uint height;

    g_DepthTexture.GetDimensions(
        width,
        height
    );

    uint sceneWidth;
    uint sceneHeight;

    g_SceneTexture.GetDimensions(
        sceneWidth,
        sceneHeight
    );


    // ================================
    // Pixel座標
    // ================================

    uint2 pixel =
        uint2(
            input.Position.x,
            input.Position.y
        );

    pixel.x =
        min(
            pixel.x,
            width - 1
        );

    pixel.y =
        min(
            pixel.y,
            height - 1
        );


    // ================================
    // 元画像
    // ================================

    float2 uv =
        input.Position.xy /
        float2(
            sceneWidth,
            sceneHeight
        );

    uv =
        saturate(uv);

    float4 sceneColor =
        g_SceneTexture.Sample(
            g_Sampler,
            uv
        );


    // ================================
    // 中央
    // ================================

    float centerDepth =
        LoadDepth(pixel);

    float3 centerNormal =
        LoadNormal(pixel);


    // ================================
    // 1px隣の座標
    // ================================

    uint2 leftPixel =
        pixel;

    leftPixel.x =
        (pixel.x > 0)
        ? pixel.x - 1
        : 0;


    uint2 rightPixel =
        pixel;

    rightPixel.x =
        min(
            pixel.x + 1,
            width - 1
        );


    uint2 upPixel =
        pixel;

    upPixel.y =
        (pixel.y > 0)
        ? pixel.y - 1
        : 0;


    uint2 downPixel =
        pixel;

    downPixel.y =
        min(
            pixel.y + 1,
            height - 1
        );


    // ================================
    // 隣の深度
    // ================================

    float leftDepth =
        LoadDepth(leftPixel);

    float rightDepth =
        LoadDepth(rightPixel);

    float upDepth =
        LoadDepth(upPixel);

    float downDepth =
        LoadDepth(downPixel);


    // ================================
    // 隣の法線
    // ================================

    float3 leftNormal =
        LoadNormal(leftPixel);

    float3 rightNormal =
        LoadNormal(rightPixel);

    float3 upNormal =
        LoadNormal(upPixel);

    float3 downNormal =
        LoadNormal(downPixel);


    // ================================
    // 深度エッジ
    // ================================

    int radius =
    clamp(
        (int) Outline.x,
        1,
        5
    );

    float depthEdge = 0.0f;

    for (int offset = 1; offset <= radius; ++offset)
    {
        uint2 leftPixel = pixel;
        leftPixel.x =
        (pixel.x >= offset)
        ? pixel.x - offset
        : 0;

        uint2 rightPixel = pixel;
        rightPixel.x =
        min(
            pixel.x + offset,
            width - 1
        );

        uint2 upPixel = pixel;
        upPixel.y =
        (pixel.y >= offset)
        ? pixel.y - offset
        : 0;

        uint2 downPixel = pixel;
        downPixel.y =
        min(
            pixel.y + offset,
            height - 1
        );

        float leftDepth = LoadDepth(leftPixel);
        float rightDepth = LoadDepth(rightPixel);
        float upDepth = LoadDepth(upPixel);
        float downDepth = LoadDepth(downPixel);

        if (centerDepth > 0.0f)
        {
            if (leftDepth <= 0.0f)
                depthEdge = 1.0f;

            if (rightDepth <= 0.0f)
                depthEdge = 1.0f;

            if (upDepth <= 0.0f)
                depthEdge = 1.0f;

            if (downDepth <= 0.0f)
                depthEdge = 1.0f;
        }
    }

    // ================================
    // 法線エッジ
    // ================================

    float normalEdge = 0.0f;

    if (centerDepth > 0.0f)
    {
        if (leftDepth > 0.0f)
        {
            normalEdge =
                max(
                    normalEdge,
                    length(
                        centerNormal -
                        leftNormal
                    )
                );
        }

        if (rightDepth > 0.0f)
        {
            normalEdge =
                max(
                    normalEdge,
                    length(
                        centerNormal -
                        rightNormal
                    )
                );
        }

        if (upDepth > 0.0f)
        {
            normalEdge =
                max(
                    normalEdge,
                    length(
                        centerNormal -
                        upNormal
                    )
                );
        }

        if (downDepth > 0.0f)
        {
            normalEdge =
                max(
                    normalEdge,
                    length(
                        centerNormal -
                        downNormal
                    )
                );
        }
    }


    // ================================
    // 深度の強度
    // ================================

    float depthSignal =
        saturate(
            depthEdge *
            Outline.y
        );


    // ================================
    // 法線強度
    //
    // 0～20 → 0～1
    // ================================

    float normalSignal =
    normalEdge * (Outline.z / 20.0f);

    float normalThreshold =
    0.20f +
    Outline.w * 0.5f;

    float normalMask =
    smoothstep(
        normalThreshold,
        normalThreshold + 0.15f,
        normalSignal
    );


    // ================================
    // 深度のしきい値
    // ================================

    float depthMask =
        smoothstep(
            Outline.w,
            Outline.w + 0.3f,
            depthSignal
        );


    // ================================
    // Depth + Normal
    // ================================

    float edge =
        max(
            depthMask,
            normalMask
        );


    // ================================
    // 黒アウトライン
    // ================================

    sceneColor.rgb =
        lerp(
            sceneColor.rgb,
            float3(
                0.0f,
                0.0f,
                0.0f
            ),
            edge
        );


    return sceneColor;
}