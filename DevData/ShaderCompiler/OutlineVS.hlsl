// OutlineVS.hlsl
// DxLib D3D11 / MV1 1FRAME—p

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
cbuffer cbOutline : register(b4)
{
    float4 OutlineSize;
};

struct VS_INPUT
{
    float4 Position : POSITION;
    float3 Normal : NORMAL0;
    float4 Diffuse : COLOR0;
    float4 Specular : COLOR1;
    float2 TexCoords0 : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : TEXCOORD0;
    float2 TexCoords0 : TEXCOORD1;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 localPosition;
    float4 worldPosition;
    float4 viewPosition;

    localPosition = input.Position;
    

    worldPosition.x = dot(
        localPosition,
        g_Base.LocalWorldMatrix[0]
    );

    worldPosition.y = dot(
        localPosition,
        g_Base.LocalWorldMatrix[1]
    );

    worldPosition.z = dot(
        localPosition,
        g_Base.LocalWorldMatrix[2]
    );

    worldPosition.w = 1.0f;

    viewPosition.x = dot(
        worldPosition,
        g_Base.ViewMatrix[0]
    );

    viewPosition.y = dot(
        worldPosition,
        g_Base.ViewMatrix[1]
    );

    viewPosition.z = dot(
        worldPosition,
        g_Base.ViewMatrix[2]
    );

    viewPosition.w = 1.0f;

    float4 clipPosition;

    clipPosition.x = dot(
    viewPosition,
    g_Base.ProjectionMatrix[0]
);

    clipPosition.y = dot(
    viewPosition,
    g_Base.ProjectionMatrix[1]
);

    clipPosition.z = dot(
    viewPosition,
    g_Base.ProjectionMatrix[2]
);

    clipPosition.w = dot(
    viewPosition,
    g_Base.ProjectionMatrix[3]
);

    float3 worldNormal;

    worldNormal.x = dot(
    input.Normal,
    g_Base.LocalWorldMatrix[0].xyz
);

    worldNormal.y = dot(
    input.Normal,
    g_Base.LocalWorldMatrix[1].xyz
);

    worldNormal.z = dot(
    input.Normal,
    g_Base.LocalWorldMatrix[2].xyz
);

    float3 viewNormal;

    viewNormal.x = dot(
    worldNormal,
    g_Base.ViewMatrix[0].xyz
);

    viewNormal.y = dot(
    worldNormal,
    g_Base.ViewMatrix[1].xyz
);

    viewNormal.z = dot(
    worldNormal,
    g_Base.ViewMatrix[2].xyz
);

    viewNormal = normalize(viewNormal);
    
    
    float2 pixelToNdc = float2(
    2.0f / OutlineSize.z,
    2.0f / OutlineSize.w
);

    clipPosition.xy += viewNormal.xy
    * OutlineSize.xy 
    * pixelToNdc
    * clipPosition.w;

    output.Position = clipPosition;

    output.Normal = input.Normal;
    output.TexCoords0 = input.TexCoords0;

    return output;
}