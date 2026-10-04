struct PS_INPUT
{
    float4 Diffuse : COLOR0;
    float4 Specular : COLOR1;
    float2 TexCoords0 : TEXCOORD0;
    float2 ToonCoords0 : TEXCOORD1;
    float4 Position : SV_POSITION;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}