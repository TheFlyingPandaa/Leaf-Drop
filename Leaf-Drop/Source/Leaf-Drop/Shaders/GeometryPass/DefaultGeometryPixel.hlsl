SamplerState defaultSampler : register(s0);


Texture2DArray textureAtlas : register(t0);
cbuffer TextureIndex : register(b0)
{
	uint4 index;
}

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

//RWStructuredBuffer<int> RayStencil : register(u0);

struct RAY_STRUCT
{
	float4	worldPos;
	uint2	pixelCoord;
};
RWStructuredBuffer<RAY_STRUCT> RayStencil : register(u0);
RWStructuredBuffer<uint> CounterStencil : register(u1);

PS_OUTPUT main(VS_OUTPUT input)
{
	PS_OUTPUT output = (PS_OUTPUT)0;
	output.position = input.worldPosition;
	output.albedo = textureAtlas.Sample(defaultSampler, float3(input.uv, index.x));

	output.normal = float4(normalize(input.normal.xyz + mul((2.0f * textureAtlas.Sample(defaultSampler, float3(input.uv, index.x + 1)).xyz - 1.0f), input.TBN)), 0);

	output.metallic = textureAtlas.Sample(defaultSampler, float3(input.uv, index.x + 2));
	
    bool CastRay = output.metallic.r > 0.9;
	float2 fIndex = float2(0.5f * input.ndc.x + 0.5f, -0.5f * input.ndc.y + 0.5f);
	int2 index = int2((int)(fIndex.x * (float)WIDTH), (int)(fIndex.y * (float)HEIGHT));

    if (CastRay && index.x > -1 && index.x < WIDTH && index.y > -1 && index.y < HEIGHT)
	{
		uint accessIndex = 0;
		//CounterStencil[0] += 1;
		InterlockedAdd(CounterStencil[0], 1u, accessIndex);
		RayStencil[accessIndex].pixelCoord = uint2(index);
		RayStencil[accessIndex].worldPos = input.worldPosition;
    }
 

	return output;
}