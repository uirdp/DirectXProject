struct VSOutput
{
	float4 svpos : SV_Position;
	float3 dir : TEXCOORD1;
};

TextureCube _CubeMap : register(t0);
SamplerState smp : register(s0);

// 定数を使うとなぜか上手くいかない・・・
// const float PI = 3.14159265359;
const float PI = 3.14;

float4 main(VSOutput input) : SV_TARGET
{
    float3 normal = normalize(input.dir);
    float3 irradiance = float3(0, 0, 0);

    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));

    float sampleDelta = 0.025;
    float nrSamples = 0.0;

    float phi_sample = 2 * 3.14159265359;
    float theta_sample = 0.5 * 3.14159265359;
    [loop]
    for (float phi = 0.0; phi < phi_sample; phi += sampleDelta)
    {
        [loop]
        for (float theta = 0.0; theta < theta_sample; theta += sampleDelta)
        {
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;

            irradiance += _CubeMap.SampleLevel(smp, sampleVec, 0).rgb * cos(theta) *sin(theta);
            nrSamples++;
        }
    }


    irradiance = 3.14159265359 * irradiance * (1.0 / max(nrSamples, 1.0));
   // irradiance = float3(1.0, 0.0, 0.0);   // this will output red
    return float4(irradiance, 1.0);
}