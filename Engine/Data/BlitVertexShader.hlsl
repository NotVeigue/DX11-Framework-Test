
struct AppData
{
	float3 Position : POSITION;
	float3 Normal   : NORMAL;
	float2 TexCoord : TEXCOORD;
};

struct VertexShaderOutput
{
	float2 TexCoord     : TEXCOORD0;
	float4 Position     : SV_Position;
};

VertexShaderOutput main(AppData IN)
{
	VertexShaderOutput OUT;

	OUT.Position = float4(IN.Position, 1.0f);//mul(WorldViewProjectionMatrix, float4(IN.Position, 1.0f));
	OUT.TexCoord = IN.TexCoord;

	return OUT;
}