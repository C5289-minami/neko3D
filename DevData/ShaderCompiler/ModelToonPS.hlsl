struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : TEXCOORD0;
    float2 UV : TEXCOORD1;
};

Texture2D g_DiffuseMap : register(t0);
SamplerState g_Sampler : register(s0);

float3 ShadowColor = float3(0.502f, 0.502f, 0.839f);

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 texColor = g_DiffuseMap.Sample(g_Sampler, input.UV);

    float3 lightDir = normalize(float3(0.3f, 1.0f, 0.5f));
    float brightness = dot(normalize(input.Normal), lightDir);

    if (brightness > 0.9f)
    {
            return float4(1, 1, 1, 1);   // 白
    }
    else if (brightness > 0.7f)
    {
        return texColor;
    }
    else if (brightness > 0.3f)
    {
        return float4(texColor.rgb * 0.7f, texColor.a);
    }
    else
    {
        // 元のテクスチャカラーと影の色を線形補間して、影の色を作成
        float3 shadow = lerp(texColor.rgb, float3(0.2f, 0.25f, 0.6f), 0.6f);
        return float4(shadow, texColor.a);
    }
}