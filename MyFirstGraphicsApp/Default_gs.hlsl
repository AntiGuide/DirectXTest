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

[maxvertexcount(15)]
void main(
    triangle GSIn               input[3],
	inout TriangleStream<GSOut> output
)
{
    for (uint k = 0; k < 5; k++)
    {
        for (uint i = 0; i < 3; i++)
        {
            float index = float(i);

            GSOut element;
            element.position = input[index].position;
            element.position.x = -1.0 + (0.05 * ((k * 3) + i));
            element.position.y /= float(k * 3) + 1.0f;
            element.color = input[index].color;
            element.texcoord = input[index].texcoord;
            output.Append(element);
        }
    
        output.RestartStrip();
    }
}