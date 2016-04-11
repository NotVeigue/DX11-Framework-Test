Texture2D Texture : register(t0);
sampler Sampler : register(s0);

struct PixelShaderInput
{
	float2 TexCoord : TEXCOORD0;
	float4 Position : SV_Position;
};

float4 main(PixelShaderInput IN) : SV_TARGET
{
	return Texture.Sample(Sampler, IN.TexCoord);
	//return float4(1.0, 0.0, 0.0, 1.0);
	//return float4(IN.TexCoord, 0.0, 1.0);
	//return float4(IN.Position.x/800.0, IN.Position.y/600.0, 0.0, 1.0);
}