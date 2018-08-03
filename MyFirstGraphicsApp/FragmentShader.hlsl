struct FSInput
{
	//float4 position : POSITION0;
	float4 color    : COLOR0;
	//float4 texcoord : TEXCOORD0;
};

float4 main(FSInput input) : SV_TARGET
{
	return input.color;
}