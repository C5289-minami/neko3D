
cbuffer ToonSettings : register(b5)
{
    float4 ShadowColor;
    float4 Thresholds;
    float4 Shadow;
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
 
    float3 lightDir = normalize(float3(0.3f, 1.0f, 0.5f));
    float brightness = dot(normalize(input.Normal), lightDir);

    if (brightness > Thresholds.x)
    {
        return float4(texColor.rgb * 1.2f, texColor.a);
    }   
    else if (brightness > Thresholds.y)
    {
        return texColor;
    }
    else if (brightness > Thresholds.z)
    {
        float3 shadow = texColor.rgb * Shadow.x;
        shadow = lerp(shadow, ShadowColor.rgb * 0.7f, Shadow.y);
        return float4(shadow, texColor.a);
    }
    else
    {
        float3 shadow = texColor.rgb * Shadow.x;
        shadow = lerp(shadow, ShadowColor.rgb, Shadow.y);
        return float4(shadow, texColor.a);
    }
}