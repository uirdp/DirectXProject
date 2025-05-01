#define PI 3.14159265359

struct VSOutput
{
    float4 svpos : SV_POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
    float4 pos : TEXCOORD1;
};

struct Light
{
    float3 Position;
    float Intensity;
};

cbuffer SceneData : register(b1)
{
    Light Lights[4];
    int LightCount;
    float3 CameraPosition;
};

inline float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = saturate(dot(N, H));
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

float4 main(VSOutput input) : SV_TARGET
{
    float3 color = float3(0.0f, 0.0f, 0.0f);
    
    for (int i = 0; i < LightCount; i++)
    {
        float3 lightDir = normalize(Lights[i].Position - input.pos.xyz);
        float intensity = saturate(dot(input.normal, lightDir));
        color += intensity;
    }

    return float4(color, 1.0f);
}