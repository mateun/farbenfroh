

struct VOut
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

cbuffer MatrixBuffer {
	matrix worldm;
	matrix viewm;
	matrix projm;
	float2 textureScale;
};

VOut VShader(float4 position : POSITION, float2 tex : TEXCOORD0) {

	VOut output;
	output.pos = mul(position, worldm);
	output.pos = mul(output.pos, viewm);
	output.pos = mul(output.pos, projm);

	output.tex = tex;
	output.tex.y = 1-output.tex.y;
	output.tex.x *= textureScale.x;
	output.tex.y *= textureScale.y;

	return output;
}

Texture2D shaderTexture;
SamplerState sampleType;

float4 PShader(VOut input) : SV_TARGET
{
    return shaderTexture.Sample(sampleType, input.tex);

}