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
    float2 ndc : NDCCOORD;
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
	output.normal = float4(normalize(input.normal.xyz + mul((2.0f * normal.Sample(defaultSampler, input.uv).xyz - 1.0f), input.TBN)), 0);
	output.albedo = albedo.Sample(defaultSampler, input.uv);
	output.metallic = metallic.Sample(defaultSampler, input.uv);
	
    bool CastRay = output.metallic.r > 0.5;

    if (CastRay)
    {
        float2 fIndex = float2(0.5f * input.ndc.x + 0.5f, -0.5f * input.ndc.y + 0.5f);
        RayStencil[(int) (fIndex.x * WIDTH_DIV) + (int) (fIndex.y * HEIGHT_DIV) * HEIGHT_DIV] = 1;
    }
    
	return output;
}