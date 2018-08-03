
struct VSInput
{
    float4 position : POSITION0;
    float4 color    : COLOR0;
    float4 texcoord : TEXCOORD0;
};

float4 main(VSInput input) : SV_POSITION
{
    return input.position;
}