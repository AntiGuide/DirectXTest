struct VSInput
{
    float4 position : POSITION0;
    float4 color    : COLOR0;
    float4 texcood  : TEXCOORD0;
};

struct VSOut
{
    float4 position : SV_POSITION;
    float4 color    : COLOR0;
    float4 texcoord : TEXCOORD0;
};

cbuffer Matrices
{
    float4x4 world;
    float4x4 view;
    float4x4 projection;
};

VSOut main(VSInput input)
{
    VSOut output;

    float4 position = mul(mul(mul(projection, view), world), input.position);

    output.position = position;
    output.color    = input.color;
    output.texcoord = input.texcood;

    return output;
}