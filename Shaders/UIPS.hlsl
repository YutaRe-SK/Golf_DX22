#include "Common.hlsli"

float4 PSMain(PS_INPUT input) : SV_TARGET
{
    return input.Color;
}