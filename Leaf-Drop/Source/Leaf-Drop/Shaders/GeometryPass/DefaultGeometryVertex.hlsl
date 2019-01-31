
cbuffer CAMERA : register(b0)
{
	float4x4 ViewProj;
}

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
	float2 uv : TEXCOORD;
};

VS_OUTPUT main(VS_INPUT input, uint instanceID : SV_InstanceID)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.position =		mul(input.position, ViewProj);
	output.worldPosition =	input.position;
	output.normal =			input.position;
	output.tangent =		input.position;
	output.biTangent =		input.position;
	output.uv =				input.uv;

	return output;
}		   
