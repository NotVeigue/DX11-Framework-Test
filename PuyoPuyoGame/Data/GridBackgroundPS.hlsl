
#define BRICK_SCALE 20.0f

struct PixelShaderInput
{
	float2 TexCoord : TEXCOORD0;
	float4 Position : SV_Position;
};

float4 main(PixelShaderInput IN) : SV_TARGET
{
	float2 uv = float2(IN.TexCoord.x * 0.7, 1.0 - IN.TexCoord.y);
	uv.x += fmod(uv.y * BRICK_SCALE, 2.0) > 1.0 ? (1.0/BRICK_SCALE) * 0.5 : 0.0;
	float2 muv = (fmod(uv * BRICK_SCALE, 1.0) - 0.5);
	float m = smoothstep(0.0, 0.75, 1.0 - max(abs(muv.x), abs(muv.y)));
	return float4(float3(0.7, 0.32, 0.35) * m, 1.0);
}