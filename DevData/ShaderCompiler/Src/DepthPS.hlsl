struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float r = input.Position.x / 1280.0f;
    float g = input.Position.y / 720.0f;

    return float4(
        saturate(r),
        saturate(g),
        0.0f,
        1.0f
    );
}