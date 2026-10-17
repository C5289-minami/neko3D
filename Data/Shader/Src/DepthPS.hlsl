
// 法線と深度を出力するピクセルシェーダ

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // 法線取得
    float3 normal =
        normalize(input.Normal);

    // 0～1に変換
    normal =
        normal * 0.5f + 0.5f;

    // 深度取得
    float depth =
        saturate(input.Position.z);

    // 法線と深度を出力
    return float4(
        normal,
        depth
    );
}