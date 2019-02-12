SamplerState defaultSampler : register(s0);
Texture2D albedo : register(t0);
Texture2D normal : register(t1);
Texture2D metallic : register(t2);


struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 worldPosition : POSITION;
	float4 normal : NORMAL;
	float4 tangent : TANGENT;
	float4 biTangent : BITANGENT;
	float3x3 TBN : TBN;
	float2 uv : TEXCOORD;
};

struct PS_OUTPUT
{
	float4 position : SV_TARGET0;
	float4 normal	: SV_TARGET1;
	float4 albedo	: SV_TARGET2;
	float4 metallic	: SV_TARGET3;
};

RWStructuredBuffer<float> RayStencil : register(u0);

PS_OUTPUT main(VS_OUTPUT input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;
	output.position = input.worldPosition;
	output.normal = float4(normalize(input.normal.xyz + mul((2.0f * normal.Sample(defaultSampler, input.uv).xyz - 1.0f), input.TBN)), 0);
	output.albedo = albedo.Sample(defaultSampler, input.uv);
	output.metallic = metallic.Sample(defaultSampler, input.uv);
	

	RayStencil[0] = WIDTH;
	RayStencil[1] = HEIGHT;
	
	return output;
}