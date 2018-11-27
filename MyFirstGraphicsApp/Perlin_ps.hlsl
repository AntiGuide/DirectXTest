struct PSIn
{
    float4 position : SV_POSITION;
    float4 color    : COLOR0;
    float4 texcoord : TEXCOORD0;
};

Texture2D    textureData[8];
SamplerState textureSampler[8];

float fade(float t)
{
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

float2 grad(float2 p)
{
    const float textureWidth = 128;

    float4 v = textureData[0].Sample(textureSampler[0], float2(p.x / textureWidth, p.y / textureWidth ));
    return normalize(2.0 * v.xy - float2(1.0, 1.0));
}

float noise(float2 p)
{
    float2 p0 = floor(p);
    float2 p1 = p0 + float2(1.0, 0.0);
    float2 p2 = p0 + float2(0.0, 1.0);
    float2 p3 = p0 + float2(1.0, 1.0);

    float delta0 = (p.x - p0.x);
    float delta2 = (p.y - p0.y);

    float fade0 = fade(delta0);
    float fade1 = fade(delta2);

    float2 g0 = grad(p0);
    float2 g1 = grad(p1);
    float2 g2 = grad(p2);
    float2 g3 = grad(p3);
    
    float gp0p1 = (((1.0 - fade0) * dot(g0, (p - p0))) + (fade0 * dot(g1, (p - p1))));
    float gp2p3 = (((1.0 - fade0) * dot(g2, (p - p2))) + (fade0 * dot(g3, (p - p3))));

    float gp0p1p2p3 = (((1.0 - fade1) * gp0p1) + (fade1 * gp2p3));

    return gp0p1p2p3;
}

float4 main(PSIn input) : SV_TARGET
{
    //return textureData[0].Sample(textureSampler[0], input.texcoord.xy);
    float color = noise(input.texcoord.xy * 1024.0 / 64.0) * 1.0 
                + noise(input.texcoord.xy * 1024.0 / 32.0) * 0.5
                + noise(input.texcoord.xy * 1024.0 / 16.0) * 0.25
                + noise(input.texcoord.xy * 1024.0 /  8.0) * 0.125;
    
    float adjusted = ((0.5 * color) + 0.5);
    return float4(adjusted.rrr, 1.0);
}