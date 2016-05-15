//TODO: FINISH WRITING THIS!!!
Texture2D DepthTexture : register(t0);
sampler Sampler : register(s0);

cbuffer SubsurfaceConstants : register(b0)
{
	float4	Color		: packoffset(c0.x);
	float3	LightPos	: packoffset(c1.x);
	float	Near		: packoffset(c1.w);
	float	Far			: packoffset(c2.x);
}

struct PixelShaderInput
{
	float4 PositionWS   : TEXCOORD1;
	float3 NormalWS     : TEXCOORD2;
	float2 TexCoord     : TEXCOORD0;
	float4 Position     : SV_Position;
};

float4 main(PixelShaderInput IN) : SV_TARGET
{
	//return Color;
	//float t = smoothstep(0.0001, 0.25, DepthTexture.Sample(Sampler, IN.Position.xy / float2(800.0, 600.0)).r - IN.Position.z);
	float3 n = normalize(IN.NormalWS);
	float t = DepthTexture.Sample(Sampler, IN.Position.xy / float2(800.0, 600.0)).r - IN.Position.z;
	float b = exp(-2.0f * t);
	float f = 1.0 - pow(1.0 - abs(dot(n, float3(0.0, 0.0, -1.0))), 4.0);
	float s = pow(saturate(dot(n, LightPos)), 64.0);
	float e = pow(1.0 - dot(n, float3(0.0, 0.0, -1.0)), 3.0);
	return float4(b * Color.xyz + s + e, f);
}