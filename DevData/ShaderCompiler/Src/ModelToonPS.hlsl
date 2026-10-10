
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
    float4 texColor = g_DiffuseMap.Sample(
        g_Sampler,
        input.UV
    );

    float3 lightDir = normalize(-LightDirection.xyz);

    float brightness = dot(
        normalize(input.Normal),
        lightDir
    );

    float3 finalColor;

    if (brightness > Thresholds.x)
    {
        finalColor = texColor.rgb * 1.2f;
    }
    else if (brightness > Thresholds.y)
    {
        finalColor = texColor.rgb;
    }
    else if (Shadow.z > 0.5f)
    {
    // ハーフトーンモード：
    // 既存の影色で暗くせず、テクスチャの色を残す
        finalColor = texColor.rgb;

    // 大きさを変えるとドットと間隔が一緒に変化する
        float spacing = max(HalftoneSettings.x, 2.0f);

        float2 cell =
        frac(input.Position.xy / spacing) - 0.5f;

        float distanceToCenter = length(cell);

        float dots = 1.0f - smoothstep(
        0.32f,
        0.40f,
        distanceToCenter
    );

    // ドットの色と不透明度を適用
        finalColor = lerp(
        finalColor,
        HalftoneColor.rgb,
        dots * saturate(HalftoneSettings.y)
    );
    }
    else if (brightness > Thresholds.z)
    {
    // 通常モード：中間の影
        float3 shadow = texColor.rgb * Shadow.x;
        shadow = lerp(
        shadow,
        ShadowColor.rgb * 0.7f,
        Shadow.y
    );
        finalColor = shadow;
    }
    else
    {
    // 通常モード：濃い影
        float3 shadow = texColor.rgb * Shadow.x;
        shadow = lerp(
        shadow,
        ShadowColor.rgb,
        Shadow.y
    );
        finalColor = shadow;
    }

    return float4(finalColor, texColor.a);
}