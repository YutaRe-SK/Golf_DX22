#include "Common.hlsli"

PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    float4 worldPos = mul(float4(input.Position, 1.0f), World);
    output.WorldPos = worldPos.xyz;
    output.Position = mul(mul(worldPos, View), Projection);
    output.Normal = normalize(mul(input.Normal, (float3x3) World));
    output.Color = input.Color * ObjectColor;
    return output;
}