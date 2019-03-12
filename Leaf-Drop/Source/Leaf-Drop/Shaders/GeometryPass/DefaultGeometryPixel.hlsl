struct RAY_STRUCT
{
    float3 startPos;
    float3 normal;
    bool dispatch;
};

SamplerState defaultSampler : register(s0);

//Texture2DArray textureAtlas : register(t0, space1);
Texture2D textureAtlas[] : register(t0, space1);

StructuredBuffer<uint4> TextureIndex : register(t1);

RWStructuredBuffer<RAY_STRUCT> RayStencil : register(u0);

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
    uint4 instanceData : INSTANCE;

};

struct PS_OUTPUT
{
	float4 position : SV_TARGET0;
	float4 normal	: SV_TARGET1;
	float4 albedo	: SV_TARGET2;
	float4 metallic	: SV_TARGET3;
};

PS_OUTPUT main(VS_OUTPUT input)
{
    PS_OUTPUT output = (PS_OUTPUT)0;
	output.position = input.worldPosition;
    output.albedo = textureAtlas[TextureIndex[0].x].Sample(defaultSampler, input.uv) * input.color;
    
    output.normal = float4(normalize(input.normal.xyz + mul((2.0f * textureAtlas[TextureIndex[0].x + 1].Sample(defaultSampler, input.uv).xyz - 1.0f), input.TBN)), 0);

    output.metallic = textureAtlas[TextureIndex[0].x + 2].Sample(defaultSampler, input.uv);
	
    bool CastRay = output.metallic.r > 0.90f;

    uint2 rayStencilIndex = uint2(input.position.xy / RAY_DIV);
    uint accessIndex = rayStencilIndex.x + rayStencilIndex.y * RAY_WIDTH;

    RayStencil[accessIndex].startPos =  input.worldPosition.xyz;
    RayStencil[accessIndex].normal =    input.normal.xyz;
    RayStencil[accessIndex].dispatch =  CastRay;
    

	return output;
}