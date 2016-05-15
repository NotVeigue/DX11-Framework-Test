cbuffer PerObject : register(b0)
{
	matrix WorldMatrix;
	matrix InverseTransposeWorldMatrix;
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
	float4 PositionCS   : SV_Position;
	float4 PositionWS   : TEXCOORD1;
	float3 NormalWS     : TEXCOORD2;
};

VertexShaderOutput main(AppData IN)
{
	VertexShaderOutput OUT;

	OUT.PositionCS = mul(WorldViewProjectionMatrix, float4(IN.Position, 1.0f));
	OUT.PositionWS = mul(WorldMatrix, float4(IN.Position, 1.0f));
	OUT.NormalWS = mul((float3x3)InverseTransposeWorldMatrix, IN.Normal);

	return OUT;
}