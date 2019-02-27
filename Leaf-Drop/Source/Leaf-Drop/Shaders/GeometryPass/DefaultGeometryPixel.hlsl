SamplerState defaultSampler : register(s0);
SamplerComparisonState depthSampler : register(s1);

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
	float4 color : COLOR;
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
	float3	startPos;
    float3  normal;
	//uint2	pixelCoord;
    bool    dispatch;
};

RWStructuredBuffer<RAY_STRUCT> RayStencil : register(u0);
RWStructuredBuffer<uint> CounterStencil : register(u1);

Texture2D depthBuffer : register(t0, space1);

PS_OUTPUT main(VS_OUTPUT input)
{
    PS_OUTPUT output = (PS_OUTPUT)0;
	output.position = input.worldPosition;
	output.albedo = textureAtlas.Sample(defaultSampler, float3(input.uv, index.x)) * input.color;

	output.normal = float4(normalize(input.normal.xyz + mul((2.0f * textureAtlas.Sample(defaultSampler, float3(input.uv, index.x + 1)).xyz - 1.0f), input.TBN)), 0);

	output.metallic = textureAtlas.Sample(defaultSampler, float3(input.uv, index.x + 2));
	
    bool CastRay = output.metallic.r > 0.15;
    
    uint2 rayStencilIndex = uint2(input.position.xy);

    uint accessIndex = rayStencilIndex.x + rayStencilIndex.y * WIDTH;

    RayStencil[accessIndex].startPos =  input.worldPosition.xyz;
    RayStencil[accessIndex].normal =    output.normal.xyz;
    RayStencil[accessIndex].dispatch =  CastRay;
    
	return output;
}