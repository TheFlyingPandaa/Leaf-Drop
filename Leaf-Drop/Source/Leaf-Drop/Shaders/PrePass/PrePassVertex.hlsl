
cbuffer CAMERA : register(b0)
{
	float4x4 ViewProj;
}

struct ObjectData
{
	float4x4	WorldMatrix;
	float4		Color;
};

StructuredBuffer<ObjectData> ObjectBuffer: register(t0);

struct VS_INPUT
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float4 biTangent : BITANGENT;
    float4 uv : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input, uint instanceID : SV_InstanceID)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	float4 worldPos =		mul(input.position, ObjectBuffer[instanceID].WorldMatrix);
	output.position =		mul(worldPos, ViewProj);


	return output;
}		   
