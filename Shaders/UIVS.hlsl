#include "Common.hlsli"

// World already contains the pixel-to-screen transform
PS_INPUT VSMain(VS_INPUT input)
{
    PS_INPUT output;
    output.Position = mul(float4(input.Position, 1.0f), World);
    output.WorldPos = float3(0.0f, 0.0f, 0.0f);
    output.Normal = float3(0.0f, 0.0f, -1.0f);
    output.Color = input.Color * ObjectColor;
    return output;
}