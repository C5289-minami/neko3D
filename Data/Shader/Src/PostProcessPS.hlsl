Texture2D g_SceneTexture : register(t0);
Texture2D g_DepthTexture : register(t1);
Texture2D g_ObjectIdTexture : register(t2);

SamplerState g_Sampler : register(s0);

#define MAX_OUTLINE_OBJECTS 256

cbuffer OutlineSettings : register(b6)
{
    // 1モデルにつき float4 × 4
    float4 OutlineSettingsTable[(MAX_OUTLINE_OBJECTS + 1) * 4];
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
};

static const float kNormalScaleFactor = 20.0f;
static const float kNormalSmoothWidth = 0.15f;

// オブジェクトIDを取得
float LoadObjectId(uint2 pixel)
{
    return g_ObjectIdTexture.Load(int3(pixel, 0)).r;
}
// 深度を取得
float LoadDepth(uint2 pixel)
{
    return g_DepthTexture.Load(int3(pixel, 0)).a;
}

// 法線を取得
float3 LoadNormal(uint2 pixel)
{
    float3 normal = g_DepthTexture.Load(int3(pixel, 0)).rgb;
    return normalize(normal * 2.0f - 1.0f);
}

// 同じオブジェクトか判定
bool IsSameObject(float a, float b)
{
    return a > 0.5f && abs(a - b) < 0.25f;
}
// 手前側のオブジェクト境界だけを輪郭として扱う
bool IsOuterBoundary(
    float centerId,
    float centerDepth,
    float sampleId,
    float sampleDepth,
    float depthStrength)
{
    // 背景との境界
    if (sampleId < 0.5f)
        return true;

    // 同じオブジェクトなら輪郭ではない
    if (IsSameObject(centerId, sampleId))
        return false;

    // IDがあるのに深度が無効なら判定しない
    if (sampleDepth <= 0.000001f)
        return false;

    // 深度検出の感度
    float depthThreshold =
        0.0001f * (20.0f / max(depthStrength, 0.001f));

    // 現在のピクセルが手前の場合だけ線を描く
    return centerDepth < sampleDepth - depthThreshold;
}


float4 main(PS_INPUT input) : SV_TARGET
{
    uint width, height;
    g_ObjectIdTexture.GetDimensions(width, height);

    uint sceneWidth, sceneHeight;
    g_SceneTexture.GetDimensions(sceneWidth, sceneHeight);

    uint2 pixel = uint2(
        min((uint) input.Position.x, width - 1),
        min((uint) input.Position.y, height - 1)
    );

    float2 uv = saturate(
        input.Position.xy / float2(sceneWidth, sceneHeight)
    );

    float4 sceneColor = g_SceneTexture.Sample(g_Sampler, uv);

    float centerId = LoadObjectId(pixel);
    float centerDepth = LoadDepth(pixel);

// 背景には線を描かず、元のシーン画像を返す
    if (centerId < 0.5f)
    {
        return sceneColor;
    }

    float3 centerNormal = LoadNormal(pixel);
    
    // Object IDから対応する設定を取得
    uint objectId = min(
    (uint) (centerId + 0.5f),
    (uint) MAX_OUTLINE_OBJECTS
);

    uint settingsIndex = objectId * 4;

    float4 OuterSettings =
    OutlineSettingsTable[settingsIndex];

    float4 InnerSettings =
    OutlineSettingsTable[settingsIndex + 1];

    float4 OutlineColor =
    OutlineSettingsTable[settingsIndex + 2];

    // ========================================
    // ① 内側：同じオブジェクト内の法線エッジ
    // ========================================
    int innerRadius = (int) (InnerSettings.x);
    float normalEdge = 0.0f;

    for (int offset = 1; offset <= innerRadius; ++offset)
    {
        uint2 lPx = uint2(
            pixel.x >= (uint) offset ? pixel.x - offset : 0,
            pixel.y
        );

        uint2 rPx = uint2(
            min(pixel.x + offset, width - 1),
            pixel.y
        );

        uint2 uPx = uint2(
            pixel.x,
            pixel.y >= (uint) offset ? pixel.y - offset : 0
        );

        uint2 dPx = uint2(
            pixel.x,
            min(pixel.y + offset, height - 1)
        );

        float lId = LoadObjectId(lPx);
        float rId = LoadObjectId(rPx);
        float uId = LoadObjectId(uPx);
        float dId = LoadObjectId(dPx);

        if (IsSameObject(centerId, lId))
        {
            normalEdge = max(
                normalEdge,
                length(centerNormal - LoadNormal(lPx))
            );
        }

        if (IsSameObject(centerId, rId))
        {
            normalEdge = max(
                normalEdge,
                length(centerNormal - LoadNormal(rPx))
            );
        }

        if (IsSameObject(centerId, uId))
        {
            normalEdge = max(
                normalEdge,
                length(centerNormal - LoadNormal(uPx))
            );
        }

        if (IsSameObject(centerId, dId))
        {
            normalEdge = max(
                normalEdge,
                length(centerNormal - LoadNormal(dPx))
            );
        }
    }

   // ========================================
// ② 外側：可視オブジェクトの輪郭
// ========================================
    int outerRadius = (int) (OuterSettings.x);
    float outerEdge = 0.0f;

    for (int offset_ = 1; offset_ <= outerRadius; ++offset_)
    {
        uint2 lPx = uint2(
        pixel.x >= (uint) offset_ ? pixel.x - offset_ : 0,
        pixel.y);

        uint2 rPx = uint2(
        min(pixel.x + offset_, width - 1),
        pixel.y);

        uint2 uPx = uint2(
        pixel.x,
        pixel.y >= (uint) offset_ ? pixel.y - offset_ : 0);

        uint2 dPx = uint2(
        pixel.x,
        min(pixel.y + offset_, height - 1));

        float lId = LoadObjectId(lPx);
        float rId = LoadObjectId(rPx);
        float uId = LoadObjectId(uPx);
        float dId = LoadObjectId(dPx);

        float lD = LoadDepth(lPx);
        float rD = LoadDepth(rPx);
        float uD = LoadDepth(uPx);
        float dD = LoadDepth(dPx);

        if (IsOuterBoundary(centerId, centerDepth, lId, lD, OuterSettings.y) ||
             IsOuterBoundary(centerId, centerDepth, rId, rD, OuterSettings.y) ||
             IsOuterBoundary(centerId, centerDepth, uId, uD, OuterSettings.y) ||
             IsOuterBoundary(centerId, centerDepth, dId, dD, OuterSettings.y))
        {
            outerEdge = 1.0f;
            break;
        }
    }
    // ========================================
    // ③ 内側・外側の線の強さ
    // ========================================

    float normalSignal =
        normalEdge * (InnerSettings.y / kNormalScaleFactor);

    float innerMask = smoothstep(
        InnerSettings.z,
        InnerSettings.z + kNormalSmoothWidth,
        normalSignal
    );

    float outerMask = outerEdge;

    // ========================================
    // ④ 不透明度を個別に適用
    // ========================================

    float outerAlpha = outerMask * saturate(OuterSettings.z);
    float innerAlpha = innerMask * saturate(InnerSettings.w);

    // アウトラインカラーのAlphaも適用
    float finalAlpha =
    max(outerAlpha, innerAlpha) * saturate(OutlineColor.a);

    sceneColor.rgb = lerp(
    sceneColor.rgb,
    OutlineColor.rgb,
    finalAlpha
    );

    return sceneColor;
}
