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

    normal = normal * 2.0f - 1.0f;

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

    uint2 pixel = uint2(
        input.Position.x,
        input.Position.y
    );

    pixel.x = min(pixel.x, width - 1);
    pixel.y = min(pixel.y, height - 1);


    // ----------------------------
    // 元画像
    // ----------------------------

    float2 uv =
        input.Position.xy /
        float2(width, height);

    float4 sceneColor =
        g_SceneTexture.Sample(
            g_Sampler,
            uv
        );


    // ----------------------------
    // 中央
    // ----------------------------

    float centerDepth =
        LoadDepth(pixel);

    float3 centerNormal =
        LoadNormal(pixel);


    // ----------------------------
    // アウトライン半径
    // ----------------------------

    int radius =
        max(1, (int)Outline.x);


    float depthEdge = 0.0f;
    float normalEdge = 0.0f;


    // ----------------------------
    // 指定された半径まで調べる
    // ----------------------------

    for (int offset = 1; offset <= radius; ++offset)
    {
        // ========================
        // 左
        // ========================

        uint2 leftPixel = pixel;

        leftPixel.x =
            (pixel.x >= offset)
            ? pixel.x - offset
            : 0;

        float leftDepth =
            LoadDepth(leftPixel);

        float3 leftNormal =
            LoadNormal(leftPixel);


        // ========================
        // 右
        // ========================

        uint2 rightPixel = pixel;

        rightPixel.x =
            min(
                pixel.x + offset,
                width - 1
            );

        float rightDepth =
            LoadDepth(rightPixel);

        float3 rightNormal =
            LoadNormal(rightPixel);


        // ========================
        // 上
        // ========================

        uint2 upPixel = pixel;

        upPixel.y =
            (pixel.y >= offset)
            ? pixel.y - offset
            : 0;

        float upDepth =
            LoadDepth(upPixel);

        float3 upNormal =
            LoadNormal(upPixel);


        // ========================
        // 下
        // ========================

        uint2 downPixel = pixel;

        downPixel.y =
            min(
                pixel.y + offset,
                height - 1
            );

        float downDepth =
            LoadDepth(downPixel);

        float3 downNormal =
            LoadNormal(downPixel);


        // ========================
        // 深度差
        // ========================

        depthEdge =
            max(
                depthEdge,
                abs(centerDepth - leftDepth)
            );

        depthEdge =
            max(
                depthEdge,
                abs(centerDepth - rightDepth)
            );

        depthEdge =
            max(
                depthEdge,
                abs(centerDepth - upDepth)
            );

        depthEdge =
            max(
                depthEdge,
                abs(centerDepth - downDepth)
            );


        // ========================
        // 法線差
        // ========================

        normalEdge =
            max(
                normalEdge,
                1.0f - dot(centerNormal, leftNormal)
            );

        normalEdge =
            max(
                normalEdge,
                1.0f - dot(centerNormal, rightNormal)
            );

        normalEdge =
            max(
                normalEdge,
                1.0f - dot(centerNormal, upNormal)
            );

        normalEdge =
            max(
                normalEdge,
                1.0f - dot(centerNormal, downNormal)
            );
    }


    // ----------------------------
    // 強度調整
    // ----------------------------

    depthEdge =
    saturate(
        depthEdge * Outline.y
    );

    normalEdge =
    saturate(
        normalEdge * Outline.z
    );


    // ----------------------------
    // Depth + Normal
    // ----------------------------

    float edge =
        max(
            depthEdge,
            normalEdge
        );


    // ----------------------------
    // しきい値
    // ----------------------------

    edge =
    smoothstep(
        Outline.w,
        Outline.w + 0.3f,
        edge
    );


    // ----------------------------
    // 黒いアウトライン
    // ----------------------------

    sceneColor.rgb =
        lerp(
            sceneColor.rgb,
            float3(0.0f, 0.0f, 0.0f),
            edge
        );

    return sceneColor;
}