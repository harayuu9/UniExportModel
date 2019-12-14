SamplerState samLinear : register(s0);

Texture2D Diffuse : register(t0);

cbuffer ConstBuff : register(b0)
{
	matrix mtxWorld;
	matrix mtxView;
	matrix mtxProj;
}

struct VS_INPUT
{
	float3 Pos : POSITION;
	float4 Col : COLOR;
	float2 Tex : TEXCOORD;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float4 Col : COL;
	float2 Tex : TEXCOORD0;
    float4 WPos : TEXCOORD1;
};

PS_INPUT vsMain(VS_INPUT pos)
{
	PS_INPUT o = (PS_INPUT)0;
	o.WPos = mul(float4(pos.Pos, 1),mtxWorld);
	o.Pos = mul(o.WPos, mtxView);
	o.Pos = mul(o.Pos, mtxProj);
	o.Col = pos.Col;
	o.Tex = pos.Tex;
	return o;
}

float4 psMain(PS_INPUT input) : SV_TARGET
{
	float4 result = 0;
	result = Diffuse.Sample(samLinear, input.Tex) * input.Col;
	return result;
}