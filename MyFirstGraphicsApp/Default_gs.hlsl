struct GSIn
{
    float4 position : SV_POSITION;
    float4 color    : COLOR0;
    float4 texcoord : TEXCOORD0;
};

struct GSOut
{
    float4 position : SV_POSITION;
    float4 color    : COLOR0;
    float4 texcoord : TEXCOORD0;
};

[maxvertexcount(6)]
void main(
    triangle GSIn               input[3],
	inout TriangleStream<GSOut> output
)
{
    for (uint k = 0; k < 2; k++)
    {
        for (uint i = 0; i < 3; i++)
        {
            float index = float(i);

            GSOut element;
            element.position = input[index].position;
            element.position.x = -1.0 + (0.3 * ((k * 3) + i));
            element.color = input[index].color;
            element.texcoord = input[index].texcoord;
            output.Append(element);
        }
    
        output.RestartStrip();
    }
}