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

SamplerState smp : register(s0);
Texture2D _MainTex : register(t0);
TextureCube _CubeMap : register(t1);

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
    float roughness = 0.5;
    float metallic = 0.5;

    float3 albedo = _MainTex.Sample(smp, input.uv);

    float3 N = normalize(input.normal);
    float3 V = normalize(CameraPosition - input.pos.xyz);
    
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);
    
    float3 Lo = float3(0, 0, 0);
    for (int i = 0; i < LightCount; i++)
    {
        float3 L = normalize(Lights[i].Position - input.pos.xyz);
        float3 H = normalize(V + L);
        
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = FresnelSchlick(saturate(dot(H, V)), F0);
        
        float3 Ks = F;
        float3 Kd = float3(1.0, 1.0, 1.0) - Ks;
        Kd *= 1.0 - metallic;
        
        float esp = 1.0e-5;
        float3 numerator = NDF * G * F;
        float denominator = 4.0 * saturate(dot(N, V)) * saturate(dot(N, L)) + esp;
        
        float3 specular = numerator / denominator;
        
        float NdotL = saturate(dot(N, L));
        Lo += (Kd * albedo / PI + specular) * NdotL;
    }

    float3 F = FresnelSchlick(saturate(dot(N, V)), F0);

    float3 Ks = F;
    float3 Kd = float3(1.0, 1.0, 1.0) - Ks;
    Kd *= 1.0 - metallic;

    float3 irradiance = _CubeMap.Sample(smp, N).rgb;
    float3 diffuse =  albedo * irradiance;
    float3 ambient = (Kd * diffuse);
    
    float3 color = Lo * albedo + ambient;
    color = color / (color + float3(1.0, 1.0, 1.0));
    color = pow(color, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    
    //scolor = N * 0.5 + 0.5;
    
    return float4(color, 1.0);

}