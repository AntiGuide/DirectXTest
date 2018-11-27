struct PSIn
{
    float4 position : SV_POSITION;
    float4 color    : COLOR0;
    float4 texcoord : TEXCOORD0;
};

Texture2D    textureData[8];
SamplerState textureSampler[8];

float4 main(PSIn input) : SV_TARGET
{
    float4 color = textureData[0].Sample(textureSampler[0], input.texcoord.xy);
    // float4 color = float4(1.0f, 0.5f, 1.0f, 1.0f);
    return color;
}