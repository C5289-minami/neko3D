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

float4 TransformPosition(float4 position)
{
    float4 world;

    world.x = dot(position, g_Base.LocalWorldMatrix[0]);
    world.y = dot(position, g_Base.LocalWorldMatrix[1]);
    world.z = dot(position, g_Base.LocalWorldMatrix[2]);
    world.w = 1.0f;

    float4 view;

    view.x = dot(world, g_Base.ViewMatrix[0]);
    view.y = dot(world, g_Base.ViewMatrix[1]);
    view.z = dot(world, g_Base.ViewMatrix[2]);
    view.w = 1.0f;

    float4 clip;

    clip.x = dot(view, g_Base.ProjectionMatrix[0]);
    clip.y = dot(view, g_Base.ProjectionMatrix[1]);
    clip.z = dot(view, g_Base.ProjectionMatrix[2]);
    clip.w = dot(view, g_Base.ProjectionMatrix[3]);

    return clip;
}

float3 TransformNormal(float3 normal)
{
    float3 result;

    result.x = dot(normal, g_Base.LocalWorldMatrix[0].xyz);
    result.y = dot(normal, g_Base.LocalWorldMatrix[1].xyz);
    result.z = dot(normal, g_Base.LocalWorldMatrix[2].xyz);

    return normalize(result);
}