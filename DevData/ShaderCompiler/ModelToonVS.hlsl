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
cbuffer TestBuffer : register(b3)
{
    float4 Bone[162];
};

struct VS_INPUT
{
    float4 Position : POSITION;
    float3 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float4 Specular : COLOR1;
    float4 TexCoords0 : TEXCOORD0;
    float4 TexCoords1 : TEXCOORD1;
    float3 Tan : TANGENT0;
    float3 Bin : BINORMAL0;
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

    float4 bone0;
    float4 bone1;
    float4 bone2;

    bone0 =
        Bone[v.BlendIndices0.x + 0] * v.BlendWeight0.x +
        Bone[v.BlendIndices0.y + 0] * v.BlendWeight0.y +
        Bone[v.BlendIndices0.z + 0] * v.BlendWeight0.z +
        Bone[v.BlendIndices0.w + 0] * v.BlendWeight0.w;

    bone1 =
        Bone[v.BlendIndices0.x + 1] * v.BlendWeight0.x +
        Bone[v.BlendIndices0.y + 1] * v.BlendWeight0.y +
        Bone[v.BlendIndices0.z + 1] * v.BlendWeight0.z +
        Bone[v.BlendIndices0.w + 1] * v.BlendWeight0.w;

    bone2 =
        Bone[v.BlendIndices0.x + 2] * v.BlendWeight0.x +
        Bone[v.BlendIndices0.y + 2] * v.BlendWeight0.y +
        Bone[v.BlendIndices0.z + 2] * v.BlendWeight0.z +
        Bone[v.BlendIndices0.w + 2] * v.BlendWeight0.w;

    float4 world;

    world.x = dot(v.Position, bone0);
    world.y = dot(v.Position, bone1);
    world.z = dot(v.Position, bone2);
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
    
    float3 worldNormal;

    worldNormal.x =
    dot(v.Normal, bone0.xyz);

    worldNormal.y =
    dot(v.Normal, bone1.xyz);

    worldNormal.z =
    dot(v.Normal, bone2.xyz);

    o.Normal = normalize(worldNormal);
    
    o.UV = v.TexCoords0.xy;
    return o;
}