struct VSOutput
{
    float4 svpos : SV_POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
    float4 pos : TEXCOORD1;
};

cbuffer SceneData : register(b1)
{
    float3 LightPositions[4];
    int LightCount;
};

float4 main(VSOutput input) : SV_TARGET
{
    float3 color = float3(0.0f, 0.0f, 0.0f);
    
    for (int i = 0; i < LightCount; i++)
    {
        float3 lightDir = normalize(LightPositions[i] - input.pos.xyz);
        float intensity = saturate(dot(input.normal, lightDir));
        color += intensity;
    }

    return float4(color, 1.0f);
}