cbuffer Transform : register(b2)
{
	float4x4 World;
	float4x4 View;
	float4x4 Proj;
	float4x4 WorldInverseTranspose;
}

struct VSInput
{
	float3 pos : POSITION;
};

struct VSOutput
{
	float4 svpos : SV_Position;
	float3 dir : TEXCOORD1;
};

VSOutput vert(VSInput input) 
{
	VSOutput output;
	float4 localPos = float4(input.pos, 1.0);
	float4 worldPos = mul(World, localPos);
	output.dir = input.pos;
	float4 viewpos = mul(View, worldPos);
	output.svpos = mul(Proj, viewpos);
	// output.dir = mul(Proj, viewpos);
	return output;
}