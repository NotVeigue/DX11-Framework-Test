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
	//return float4(IN.NormalWS, 1.0);
	//return PixelColor;
	// c = sin(dot(IN.NormalWS, float3(0.0, 1.0, 0.0)) * 3.14 * 16.0);
	//return float4(c, c, c, 1.0);
	return PixelColor;
}