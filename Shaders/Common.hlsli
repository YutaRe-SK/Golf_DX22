// Shared definitions
cbuffer FrameBuffer : register(b0)
{
    matrix View;
    matrix Projection;
    float4 LightDir; // xyz: direction the light travels
    float4 CameraPos;
};

cbuffer ObjectBuffer : register(b1)
{
    matrix World;
    float4 ObjectColor;
};

struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
};

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 WorldPos : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float4 Color : COLOR;
};