float4 baseLightingCoords : register(c8);
float4 fogConsts : register(c21);
float4x4 shadowLookupMatrix : register(c24);
float4x4 viewProjectionMatrix : register(c0);
float4x4 worldMatrix : register(c4);


void main(
	in float4 pos : POSITION,
	in float4 normal : NORMAL,
	in half2 uv0 : TEXCOORD0,
	in float4 color : COLOR,

	out float4 pos_out : POSITION,
	out float4 color_out : COLOR0,
	out float4 out00 : TEXCOORD0,
	out float4 out01 : TEXCOORD1
)
{
	pos_out = mul(pos, worldMatrix);
	pos_out = mul(pos_out, viewProjectionMatrix);

	color_out = color;

	out00 = float4(uv0.x, uv0.y, 0, 0);
	out01 = normal, 1;
}