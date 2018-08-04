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

VSOut main(VSInput input)
{
    VSOut output;
    output.position = input.position;
    output.color    = input.color;
    output.texcoord = input.texcood;

    return output;
}