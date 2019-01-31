SamplerState defaultSampler : register(s0);
Texture2D albedo : register(t0);


struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 worldPosition : POSITION;
	float4 normal : NORMAL;
	float4 tangent : TANGENT;
	float4 biTangent : BITANGENT;
	float2 uv : TEXCOORD;
};

float4 main(VS_OUTPUT input) : SV_TARGET
{
	return albedo.Sample(defaultSampler, input.uv);
}