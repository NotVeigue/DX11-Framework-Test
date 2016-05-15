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
}