

struct VOut 
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 norm : NORMAL;
};

cbuffer MatrixBuffer {
	matrix worldm;
	matrix viewm;
	matrix projm;
	float2 textureScale;
};

VOut VShader(float4 position : POSITION, float2 tex : TEXCOORD0, float3 norm : NORMAL) {

	VOut output;
	output.pos = mul(position, worldm);
	output.pos = mul(output.pos, viewm);
	output.pos = mul(output.pos, projm);
	//output.pos = float4(position.x, position.y, position.z, 1);
	output.tex = tex;
	output.tex.y = 1-  output.tex.y;
	output.tex.x *= textureScale.x;
	output.tex.y *= textureScale.y;

    float4x4 worldInverseTranspose = transpose(worldm);
    output.norm = normalize(mul(norm, (float3x3)worldInverseTranspose));

	
	return output;
}

Texture2D shaderTexture;
SamplerState sampleType;

float4 PShader(VOut input) : SV_TARGET
{

	float4 textureColor = shaderTexture.Sample(sampleType, input.tex);
	//textureColor = float4(input.norm.x, input.norm.y, input.norm.z, 1);

	textureColor = textureColor * dot(input.norm, float3(0.4, 0.8, 0.2));

	return textureColor;

}