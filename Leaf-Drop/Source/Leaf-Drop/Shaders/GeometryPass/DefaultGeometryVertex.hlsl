
cbuffer CAMERA : register(b0)
{
	float4x4 ViewProj;
}

struct ObjectData
{
	float4x4	WorldMatrix;
	float4		Color;

};

StructuredBuffer<ObjectData> ObjectBuffer : register(t0);

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
	float4 color : COLOR;
};

VS_OUTPUT main(VS_INPUT input, uint instanceID : SV_InstanceID)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.worldPosition =	mul(input.position, ObjectBuffer[instanceID].WorldMatrix);
	output.position =		mul(output.worldPosition, ViewProj);

	output.normal =			normalize(mul(input.normal, ObjectBuffer[instanceID].WorldMatrix));
	output.tangent =		input.tangent;
	output.biTangent =		input.biTangent;
	output.uv =				input.uv;
    output.ndc =            output.position.xy / output.position.w;
	output.color =			ObjectBuffer[instanceID].Color;

	float3 tangent = normalize(mul(input.tangent, ObjectBuffer[instanceID].WorldMatrix).xyz);
	tangent = normalize(tangent - dot(tangent, output.normal.xyz) * output.normal.xyz).xyz;
	float3 bitangent = cross(output.normal.xyz, tangent);
	output.TBN = float3x3(tangent, bitangent, output.normal.xyz);


	return output;
}		   
