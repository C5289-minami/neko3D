#include "ModelToonVS_Common.hlsli"

// ModelToonVS_4Frame

struct VS_INPUT
{
    float4 Position : POSITION;
    float3 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float4 Specular : COLOR1;
    float4 TexCoords0 : TEXCOORD0;
    float4 TexCoords1 : TEXCOORD1;

    int4 BlendIndices0 : BLENDINDICES0;
    float4 BlendWeight0 : BLENDWEIGHT0;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : TEXCOORD0;
    float2 UV : TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT v)
{
    VS_OUTPUT o;

    o.Position = TransformPosition(v.Position);
    o.Normal = TransformNormal(v.Normal);
    o.UV = v.TexCoords0.xy;

    return o;
}