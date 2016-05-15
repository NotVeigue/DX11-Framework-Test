cbuffer VoronoiConstants : register(b0)
{
	float3 Color;
	float Scale;
	float TileScale;
}

struct PixelShaderInput
{
	float2 TexCoord : TEXCOORD0;
	float4 Position : SV_Position;
};

float2 hash22(float2 p)
{
	p = fmod(p, TileScale);
	float r = 523.0*sin(dot(p, float2(53.3158, 43.6143)));
	return float2(frac(15.32354 * r), frac(17.25865 * r));
}

float voronoi(float2 p)
{
	float2 n = floor(p);
	float2 f = p - n;

	float2 mg, mv;

	float md = 99.0;
	for (int i = -1; i <= 1; i++)
		for (int j = -1; j <= 1; j++)
		{
			float2 g = float2(float(i), float(j));
			float2 o = hash22(n + g);
			float2 v = g + o - f;

			float d = dot(v, v);
			if (d < md)
			{
				md = d;
				mg = g;
				mv = v;
			}
		}

	md = 99.0;
	for (int i = -2; i <= 2; i++)
		for (int j = -2; j <= 2; j++)
		{
			if (i == 0 && j == 0)
				continue;

			float2 g = mg + float2(float(i), float(j));
			float2 o = hash22(n + g);
			float2 v = g + o - f;

			md = min(md, dot(0.5 * (v + mv), normalize(v - mv)));
		}

	return smoothstep(-0.5, .7, md);
}

float4 main(PixelShaderInput IN) : SV_TARGET
{
	IN.TexCoord.x *= (800.0 / 600.0);
	IN.TexCoord *= Scale;

	float2 e = float2(0.00005, 0.0);
	float f = voronoi(IN.TexCoord);
	float2 gr = (float2(voronoi(IN.TexCoord + e.xy).x - voronoi(IN.TexCoord - e.xy).x, voronoi(IN.TexCoord + e.yx).x - voronoi(IN.TexCoord - e.yx).x) / (e.x * 2.0));
	float3 N = normalize(float3(gr, 0.9));
	float3 L = normalize(float3(0.0, 2.0, 0.35));
	float d = saturate(dot(N, L));

	return float4(Color * f * (0.3 + d * 0.7), 1.0);//float4(0.9, 0.5, 0.3, 1.0) * saturate(f);
}