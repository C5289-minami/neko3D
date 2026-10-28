// 3DModel用トゥーンシェーダ

cbuffer ToonSettings : register(b5)
{
    float4 ShadowColor;
    float4 Thresholds;
    float4 Shadow;
    float4 LightDirection;
    float4 HalftoneColor;
    float4 HalftoneSettings;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : TEXCOORD0;
    float2 UV : TEXCOORD1;
};

Texture2D g_DiffuseMap : register(t0);
SamplerState g_Sampler : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 texColor = g_DiffuseMap.Sample(g_Sampler, input.UV);
    float3 normal = normalize(input.Normal);
    float3 lightDir = normalize(-LightDirection.xyz);

    // 明るさ計算（環境光として全体を少し底上げし、壁が暗くなりすぎるのを防ぐ）
    float brightness = dot(normal, lightDir) * 0.5f + 0.5f;

    float3 finalColor;

    if (brightness > Thresholds.x)
    {
        // ハイライト（白飛びしないよう 1.05倍 に抑える）
        finalColor = texColor.rgb * 1.05f;
    }
    else if (brightness > Thresholds.y)
    {
        // 通常色
        finalColor = texColor.rgb;
    }
    else if (Shadow.z > 0.5f)
    {
        // ハーフトーン影
        finalColor = texColor.rgb;

        float spacing = max(HalftoneSettings.x, 2.0f);
        float2 cell = frac(input.Position.xy / spacing) - 0.5f;
        float distanceToCenter = length(cell);
        float dots = 1.0f - smoothstep(0.32f, 0.40f, distanceToCenter);

        finalColor = lerp(
            finalColor,
            HalftoneColor.rgb,
            dots * saturate(HalftoneSettings.y)
        );
    }
    else if (brightness > Thresholds.z)
    {
        // 中間の影
        float3 shadow = texColor.rgb * Shadow.x;
        shadow = lerp(shadow, ShadowColor.rgb * 0.7f, Shadow.y);
        finalColor = shadow;
    }
    else
    {
        // 濃い影
        float3 shadow = texColor.rgb * Shadow.x;
        shadow = lerp(shadow, ShadowColor.rgb, Shadow.y);
        finalColor = shadow;
    }

    return float4(saturate(finalColor), texColor.a);
}