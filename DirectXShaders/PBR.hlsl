struct VSOutput
{
    float4 svpos : SV_POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

float4 main(VSOutput input) : SV_TARGET
{
    return float4((input.normal + 1.0f) * 0.5f, 1.0f);
}