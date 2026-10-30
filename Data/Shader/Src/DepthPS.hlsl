// 深度と法線を出力するピクセルシェーダー

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 normal = normalize(input.Normal) * 0.5f + 0.5f;

    return float4(normal, saturate(input.Position.z));
}