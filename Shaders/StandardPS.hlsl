#include "Common.hlsli"

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    float3 N = normalize(input.Normal);
    float3 L = normalize(-LightDir.xyz);
    float3 V = normalize(CameraPos.xyz - input.WorldPos);
    float3 H = normalize(L + V);

    float diffuse = saturate(dot(N, L));
    float specular = pow(saturate(dot(N, H)), 32.0f) * 0.3f;
    float3 ambient = 0.3f;

    float3 color = input.Color.rgb * (ambient + diffuse) + specular;
    return float4(color, input.Color.a);
}