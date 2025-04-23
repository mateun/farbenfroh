cbuffer ObjectBuffer : register(b0)
{
    float4x4 worldViewProj;
};

struct VSPosUVInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;

};

struct VSPosInput
{
    float3 position : POSITION;

};

struct VSOutput
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

VSOutput VSMain(VSPosUVInput input)
{
    VSOutput output;
    output.pos = mul(float4(input.position, 1.0), worldViewProj);
    output.uv = input.uv;
    return output;
}

VSOutput VSPosMain(VSPosInput input) {
   VSOutput output;
    output.pos = mul(float4(input.position, 1.0), worldViewProj);
    return output;
}


Texture2D tex : register(t0);
SamplerState samp : register(s0);

cbuffer MaterialBuffer : register(b1)
{
    float4 baseColor;  // Used when no texture bound
};

float4 PSColorOnly(VSPosInput input) : SV_Target {
    return baseColor;
}

float4 PSMain(VSOutput input) : SV_Target
{
    float4 color = tex.Sample(samp, input.uv);
    return color;
}
