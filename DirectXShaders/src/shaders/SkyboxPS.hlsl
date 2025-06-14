struct VSOutput
{
	float4 svpos : SV_Position;
	float3 dir : TEXCOORD1;
};

TextureCube _CubeMap : register(t0);
SamplerState smp : register(s0);


float4 main(VSOutput input) : SV_TARGET
{
	return _CubeMap.Sample(smp, input.dir.xyz);

	// return float4(normalize(input.dir.xyz) * 0.5 + 0.5, 1.0);
}