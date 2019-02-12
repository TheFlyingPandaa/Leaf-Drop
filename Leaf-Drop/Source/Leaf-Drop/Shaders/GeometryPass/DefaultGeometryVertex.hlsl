
cbuffer CAMERA : register(b0)
{
	float4x4 ViewProj;
}

StructuredBuffer<float4x4> WorldMatrix : register(t0);

struct VS_INPUT
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 tangent : TANGENT;
	float4 biTangent : BITANGENT;
	float2 uv : TEXCOORD;
};

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

VS_OUTPUT main(VS_INPUT input, uint instanceID : SV_InstanceID)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.worldPosition =	mul(input.position, WorldMatrix[instanceID]);
	output.position =		mul(output.worldPosition, ViewProj);

	output.normal =			normalize(mul(input.normal, WorldMatrix[instanceID]));
	output.tangent =		input.tangent;
	output.biTangent =		input.biTangent;
	output.uv =				input.uv;
    output.ndc =            output.position.xy / output.position.w;

	float3 tangent = normalize(mul(input.tangent, WorldMatrix[instanceID]).xyz);
	tangent = normalize(tangent - dot(tangent, output.normal.xyz) * output.normal.xyz).xyz;
	float3 bitangent = cross(output.normal.xyz, tangent);
	output.TBN = float3x3(tangent, bitangent, output.normal.xyz);


	return output;
}		   
