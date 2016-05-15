cbuffer DepthOnlyConstants : register(b0)
{
	matrix WorldViewProjectionMatrix;
}

struct AppData
{
	float3 Position : POSITION;
	float3 Normal   : NORMAL;
	float2 TexCoord : TEXCOORD;
};

struct VertexShaderOutput
{
	float4 Position : SV_Position;
};

VertexShaderOutput main(AppData IN)
{
	VertexShaderOutput OUT;
	OUT.Position = mul(WorldViewProjectionMatrix, float4(IN.Position, 1.0f));

	return OUT;
}