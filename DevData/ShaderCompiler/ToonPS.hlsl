// ToonPS.hlsl

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : TEXCOORD0;
    float2 TexCoords0 : TEXCOORD1;
};

Texture2D g_DiffuseMap : register(t0);
SamplerState g_Sampler : register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
    
    float4 texColor = g_DiffuseMap.Sample(
    g_Sampler,
    input.TexCoords0
);
    
    float3 lightDir = normalize(float3(-0.3f, -1.0f, -0.5f));

    float brightness = dot(
        normalize(input.Normal),
            -lightDir
    );

    // 明るさの閾値を設定    
    float3 ShadowColor = float3(0.502f, 0.502f, 0.839f);
    float BrightThreshold = 0.7f;
    float MiddleThreshold = 0.3f;

    if (brightness > 0.9f)
    {
    // ハイライト
        return float4(texColor.rgb * 1.2f, texColor.a);
    }
    else if (brightness > BrightThreshold)
    {
    // 明るい
        return texColor;
    }
    else if (brightness > MiddleThreshold)
    {
    // 中間
        return float4(texColor.rgb * 0.7f, texColor.a);
    }
    else
    {
    // 影
        return float4(texColor.rgb * ShadowColor, texColor.a);
    }

}