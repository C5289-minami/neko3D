struct DX_D3D11_VS_CONST_BUFFER_BASE
{
    float4 AntiViewportMatrix[4];
    float4 ProjectionMatrix[4];
    float4 ViewMatrix[3];
    float4 LocalWorldMatrix[3];

    float4 ToonOutLineSize;

    float DiffuseSource;
    float SpecularSource;
    float MulSpecularColor;
    float Padding;
};

cbuffer cbD3D11_CONST_BUFFER_VS_BASE : register(b1)
{
    DX_D3D11_VS_CONST_BUFFER_BASE g_Base;
};


struct VS_INPUT
{
    float4 Position : POSITION;

    float3 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float4 Specular : COLOR1;
    float4 TexCoords0 : TEXCOORD0;
    float4 TexCoords1 : TEXCOORD1;
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

    float4 world;

    world.x = dot(v.Position, g_Base.LocalWorldMatrix[0]);
    world.y = dot(v.Position, g_Base.LocalWorldMatrix[1]);
    world.z = dot(v.Position, g_Base.LocalWorldMatrix[2]);
    world.w = 1.0f;

    float4 view;

    view.x = dot(world, g_Base.ViewMatrix[0]);
    view.y = dot(world, g_Base.ViewMatrix[1]);
    view.z = dot(world, g_Base.ViewMatrix[2]);
    view.w = 1.0f;

    o.Position.x = dot(view, g_Base.ProjectionMatrix[0]);
    o.Position.y = dot(view, g_Base.ProjectionMatrix[1]);
    o.Position.z = dot(view, g_Base.ProjectionMatrix[2]);
    o.Position.w = dot(view, g_Base.ProjectionMatrix[3]);

    o.Normal = v.Normal;
    o.UV = v.TexCoords0.xy;

    return o;
}