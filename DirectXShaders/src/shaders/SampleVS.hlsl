cbuffer Transform : register(b0)
{
    float4x4 World;
    float4x4 View;
    float4x4 Proj;
    float4x4 WorldInverseTranspose;
}

struct VSInput
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
    float4 color : COLOR;
};

struct VSOutput
{
    float4 svpos : SV_Position;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
    float4 pos : TEXCOORD1;
};

VSOutput vert(VSInput input)
{
    VSOutput output;
    
    float4 localPos = float4(input.pos, 1.0f);
    float4 worldPos = mul(World, localPos);
    float4 viewPos = mul(View, worldPos);
    float4 projPos = mul(Proj, viewPos);
    
    output.svpos = projPos;
    
    float4 localNormal = float4(input.normal, 0.0);
    float4 worldNormal = mul(WorldInverseTranspose, localNormal);
    
    output.normal = normalize(worldNormal); // ワールドノーマルに変更する必要がある？
    output.color = input.color;
    output.uv = input.uv;
    output.pos = projPos;
    
    return output;
}