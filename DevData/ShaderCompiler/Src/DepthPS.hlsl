struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 normal =
        normalize(input.Normal);

    normal =
        normal * 0.5f + 0.5f;

    float depth =
        saturate(input.Position.z);

    return float4(
        normal,
        depth
    );
}