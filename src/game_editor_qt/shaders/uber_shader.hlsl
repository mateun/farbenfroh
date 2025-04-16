cbuffer ObjectBuffer : register(b0)
{
    float4x4 worldViewProj;
};

struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0; // optional use
};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    output.pos = mul(float4(input.position, 1.0), worldViewProj);
    output.uv = input.uv;
    return output;
}

Texture2D tex : register(t0);
SamplerState samp : register(s0);

cbuffer MaterialBuffer : register(b1)
{
    float4 baseColor;  // Used when no texture bound
    bool useTexture;
    float3 padding;    // Align to 16 bytes
};

float4 PSMain(VSOutput input) : SV_Target
{
    float4 color = baseColor;

    if (useTexture)
    {
        color *= tex.Sample(samp, input.uv);
    }

    return color;
}
