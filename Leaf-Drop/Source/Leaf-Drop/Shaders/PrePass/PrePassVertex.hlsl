
cbuffer CAMERA : register(b0)
{
	float4x4 ViewProj;
}

StructuredBuffer<float4x4> WorldMatrix : register(t0);

struct VS_INPUT
{
	float4 position : POSITION;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input, uint instanceID : SV_InstanceID)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	float4 worldPos =		mul(input.position, WorldMatrix[instanceID]);
	output.position =		mul(worldPos, ViewProj);


	return output;
}		   
