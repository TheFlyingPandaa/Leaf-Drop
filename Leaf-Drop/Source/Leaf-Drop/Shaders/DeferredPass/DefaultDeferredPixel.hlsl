#include "../LightCalculations.hlsli"
SamplerState defaultSampler : register(s0);

Texture2D Position	: register(t0, space0);
Texture2D Normal    : register(t0, space1);
Texture2D Albedo    : register(t0, space2);
Texture2D Metallic  : register(t0, space3);


StructuredBuffer<LIGHT> Lights : register(t1);

cbuffer Camera : register (b0)
{
	float4 CameraDirection;
	float4 CameraPosition;
}

cbuffer LightSize : register (b1)
{
	uint4 LightValues;
}

struct PS_INPUT
{
	float4 position : SV_Position;
	float2 uv : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 worldPos = Position.Sample(defaultSampler, input.uv);
	float4 normal	= Normal.Sample(defaultSampler, input.uv);
	float4 albedo	= Albedo.Sample(defaultSampler, input.uv);
	float4 metallic = Metallic.Sample(defaultSampler, input.uv);

	float4 ambient = float4(0.15f, 0.15f, 0.15f, 1.0f) * albedo;

	uint numStructs;
	uint stride;
	Lights.GetDimensions(numStructs, stride);
	

	float4 specular = float4(0,0,0,1);
	float4 finalColor = float4(0,0,0,1);

	for	(uint i = 0; i < LightValues.x && i < 256; i++)
	{
		finalColor += LightCalculations(Lights[i],
			CameraPosition,
			worldPos,
			albedo,
			normal,
			metallic,
			specular);
	}


	return saturate(finalColor + ambient);
}