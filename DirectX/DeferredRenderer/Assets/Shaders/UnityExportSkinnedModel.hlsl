#include "CorePBR.hlsli"

SamplerState samLinear : register(s0);

Texture2D Diffuse : register(t0);

cbuffer ConstBuff : register(b0)
{
	matrix mtxWorld;
	matrix mtxView;
	matrix mtxProj;
}

cbuffer BoneMatrix : register(b1)
{
    matrix bone[200];
}

struct VS_INPUT
{
	float3 Pos : POSITION;
	float3 Nor : NORMAL;
	float2 Tex : TEXCOORD;
    uint4 boneIndex : BONEINDEX;
    float4 boneWeight : BONEWEIGHT;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float3 Nor : NORMAL;
	float2 Tex : TEXCOORD0;
    float3 ViewDirection : TEXCOORD1;
};

PS_INPUT vsMain(VS_INPUT pos)
{
	PS_INPUT o = (PS_INPUT)0;

    float4x4 animMtx = (float4x4) 0;
    
    for (int i = 0; i < 4;i++)
    {
        animMtx += bone[pos.boneIndex[i]] * pos.boneWeight[i];
    }

    o.Pos = float4(pos.Pos, 1);
    o.Pos = mul(o.Pos, animMtx);
    o.Pos = mul(o.Pos, mtxWorld);
    o.ViewDirection = normalize(o.Pos.xyz / o.Pos.w - mtxView._41_42_43);
	o.Pos = mul(o.Pos, mtxView);
	o.Pos = mul(o.Pos, mtxProj);
    o.Pos.xyz /= o.Pos.w;
    o.Pos.w = 1;
	o.Tex = pos.Tex;

    float3x3 rotWorld = float3x3(mtxWorld._11_12_13, mtxWorld._21_22_23, mtxWorld._31_32_33);
    o.Nor = mul(pos.Nor, float3x3(animMtx._11_12_13, animMtx._21_22_23, animMtx._31_32_33));
    o.Nor = normalize(mul(o.Nor, rotWorld));
	return o;
}

float4 psMain(PS_INPUT input) : SV_TARGET
{
	float4 result = 0;
    float4 albedo = Diffuse.Sample(samLinear, input.Tex);
    clip(albedo.a - 0.5);
    
    InputData data = (InputData)0;
    data.normalWS = input.Nor;
    data.viewDirectionWS = input.ViewDirection;

    result = LightweightFragmentPBR(data, albedo.rgb, 0.0, 0.4, 1, half3(0, 0, 0), albedo.a);
    return float4(result);
}