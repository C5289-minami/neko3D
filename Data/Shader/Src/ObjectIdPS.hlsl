// オブジェクトID出力用ピクセルシェーダー

cbuffer ObjectIdSettings : register(b7)
{
    float4 ObjectSettings; // x: Object ID, yzw: padding
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // RにオブジェクトID、Alphaは不透明に固定
    return float4(ObjectSettings.x, 0.0f, 0.0f, 1.0f);
}