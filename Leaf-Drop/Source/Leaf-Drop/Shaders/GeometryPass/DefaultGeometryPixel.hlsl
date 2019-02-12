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

struct PS_OUTPUT
{
	float4 position : SV_TARGET0;
	float4 normal	: SV_TARGET1;
	float4 albedo	: SV_TARGET2;
	float4 metallic	: SV_TARGET3;
};

RWStructuredBuffer<int> RayStencil : register(u0);

PS_OUTPUT main(VS_OUTPUT input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;
	output.position = input.worldPosition;
	output.normal = input.normal;
	output.albedo = albedo.Sample(defaultSampler, input.uv);
	output.metallic = float4(1, 1, 1, 1);

    float2 fIndex = (output.position.xy + float2(1.0f, 1.0f)) * 0.5f;

    RayStencil[(int) (fIndex.x * WIDTH_DIV) + (int) (fIndex.y * HEIGHT_DIV) * HEIGHT_DIV] = length(output.metallic) > 0.5f;
	
	return output;
}