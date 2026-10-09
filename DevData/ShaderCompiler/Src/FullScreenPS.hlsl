Texture2D g_SceneTexture : register(t0);
SamplerState g_Sampler : register(s0);

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 DiffuseColor : COLOR0;
    float4 SpecularColor : COLOR1;
    float2 TexCoords0 : TEXCOORD0;
    float2 TexCoords1 : TEXCOORD1;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    // 元のシーン画像を取得
    float4 color = g_SceneTexture.Sample(
        g_Sampler,
        input.TexCoords0
    );

    // 輝度を計算
    float luminance = dot(
        color.rgb,
        float3(0.299f, 0.587f, 0.114f)
    );

    // 画面のピクセル座標を取得
    uint width;
    uint height;
    g_SceneTexture.GetDimensions(width, height);

    float2 pixelPosition =
        input.TexCoords0 * float2(width, height);

    // ドットの間隔（ピクセル単位）
    const float dotSpacing = 7.0f;

    // 各ドットの中心からの距離を計算
    float2 cellPosition =
        frac(pixelPosition / dotSpacing) - 0.5f;

    float distanceToCenter = length(cellPosition);

    // 暗い部分ほどドットを大きくする
    float radius = lerp(
        0.3f,
        0.10f,
        luminance
    );

    // ドットの輪郭を少し滑らかにする
    float dots = 1.0f - smoothstep(
        radius,
        radius + 0.04f,
        distanceToCenter
    );
    // この明るさ以下にだけドットを表示
    const float dotThreshold = 0.45f;

// かなり暗い部分だけにドットを表示
    float shadowMask = 1.0f - smoothstep(
    0.20f,
    0.32f,
    luminance
);

    dots *= shadowMask;
    // 黒いインクを重ねる
    const float dotStrength = 0.75f;

    color.rgb = lerp(
        color.rgb,
        float3(0.0f, 0.0f, 0.0f),
        dots * dotStrength
    );

    return float4(color.rgb, 1.0f);
}