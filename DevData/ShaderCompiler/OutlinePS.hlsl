// OutlinePS.hlsl

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : TEXCOORD0;
    float2 TexCoords0 : TEXCOORD1;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    return float4(0.0f, 0.0f, 0.0f, 0.7f);
}