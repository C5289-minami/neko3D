Texture2D g_DepthTexture : register(t0);

struct PS_INPUT
{
    float4 Position : SV_POSITION;
};

float3 LoadNormal(Texture2D textureData, uint2 pixel)
{
    float3 normal =
        textureData.Load(
            int3(pixel, 0)
        ).rgb;

    // 0Å`1 Å® -1Å`1
    normal = normal * 2.0f - 1.0f;

    return normalize(normal);
}

float4 main(PS_INPUT input) : SV_TARGET
{
    uint width;
    uint height;

    g_DepthTexture.GetDimensions(width, height);

    uint2 pixel = uint2(
        input.Position.x,
        input.Position.y
    );

    pixel.x = min(pixel.x, width - 1);
    pixel.y = min(pixel.y, height - 1);

    // íÜâõÇÃñ@ê¸
    float3 centerNormal =
        LoadNormal(
            g_DepthTexture,
            pixel
        );

    // ç∂
    uint2 leftPixel = pixel;
    leftPixel.x =
        (pixel.x > 0)
        ? pixel.x - 1
        : 0;

    float3 leftNormal =
        LoadNormal(
            g_DepthTexture,
            leftPixel
        );

    // âE
    uint2 rightPixel = pixel;
    rightPixel.x =
        min(pixel.x + 1, width - 1);

    float3 rightNormal =
        LoadNormal(
            g_DepthTexture,
            rightPixel
        );

    // è„
    uint2 upPixel = pixel;
    upPixel.y =
        (pixel.y > 0)
        ? pixel.y - 1
        : 0;

    float3 upNormal =
        LoadNormal(
            g_DepthTexture,
            upPixel
        );

    // â∫
    uint2 downPixel = pixel;
    downPixel.y =
        min(pixel.y + 1, height - 1);

    float3 downNormal =
        LoadNormal(
            g_DepthTexture,
            downPixel
        );

    // ñ@ê¸ÇÃç∑
    float leftDifference =
        length(centerNormal - leftNormal);

    float rightDifference =
        length(centerNormal - rightNormal);

    float upDifference =
        length(centerNormal - upNormal);

    float downDifference =
        length(centerNormal - downNormal);

    // 4ï˚å¸ÇÃÇ§Çøç≈ëÂÇÃç∑
    float edge =
        max(
            max(leftDifference, rightDifference),
            max(upDifference, downDifference)
        );

    // å©Ç‚Ç∑Ç≠Ç∑ÇÈ
    edge = saturate(edge * 3.0f);

    return float4(
        edge,
        0.0f,
        0.0f,
        1.0f
    );
}