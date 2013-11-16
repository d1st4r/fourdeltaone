sampler2D colorSampler : register(s0);
sampler2D normalSampler : register(s5);
sampler2D specularSampler : register(s6);

float4 main(
	in float4 in00 : TEXCOORD0,
	in float4 in01 : TEXCOORD1,
	in float4 color : COLOR0
) : COLOR0
{
	return tex2D(colorSampler, in00.xy) * color;
}