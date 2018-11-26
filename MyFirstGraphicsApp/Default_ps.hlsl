struct PSIn
{
    float4 position : SV_POSITION;
    float4 color    : COLOR0;
    float4 texcoord : TEXCOORD0;
};

Texture2D    textureData;
SamplerState textureSampler;

float4 main(PSIn input) : SV_TARGET
{
    float4 color = textureData.Sample(textureSampler, input.texcoord.xy);
    return color;
}