
cbuffer SingleColor : register(b0)
{
	float4 PixelColor;
}

struct PixelShaderInput
{
	float4 PositionWS   : TEXCOORD1;
	float3 NormalWS     : TEXCOORD2;
	float2 TexCoord     : TEXCOORD0;
};

float4 main(PixelShaderInput IN) : SV_TARGET
{
	
	return PixelColor;
}