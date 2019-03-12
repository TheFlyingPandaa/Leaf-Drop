#include "../LightCalculations.hlsli"
SamplerState defaultSampler : register(s0);

Texture2D Position		: register(t0, space0);
Texture2D Normal		: register(t0, space1);
Texture2D Albedo		: register(t0, space2);
Texture2D Metallic		: register(t0, space3);
Texture2D RayTracing	: register(t0, space4);


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

float multiplier(int2 pos, int radius, float centerStregth = 4.0f)
{
    float l = length(float2(pos.x, pos.y));
    return abs(radius - l) * centerStregth;
}

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 worldPos = Position.Sample(defaultSampler, input.uv);
	float4 normal	= Normal.Sample(defaultSampler, input.uv);
	float4 albedo	= Albedo.Sample(defaultSampler, input.uv);
	float4 metallic = Metallic.Sample(defaultSampler, input.uv);
	//float4 rays2		= RayTracing.Sample(defaultSampler, input.uv);
    //return rays2;
    //return float4(worldPos);
    float width, height, element;
    RayTracing.GetDimensions(0, width, height, element);
    

    float2 texelSize = float2(1.0f / width, 1.0f / height);
    int sampleRadius = 1;

    float2 smTex;
    
    float4 rays = float4(0, 0, 0, 1);
    float divider = 0.0f;
    for (int x = -sampleRadius; x <= sampleRadius; ++x)
    {
        for (int y = -sampleRadius; y <= sampleRadius; ++y)
        {
            smTex = input.uv + (float2(x, y) * texelSize);
            rays += RayTracing.Sample(defaultSampler, smTex) * multiplier(int2(x, y), sampleRadius, 4.0f * metallic.r);
            //rays += RayTracing.Sample(defaultSampler, smTex) * multiplier(int2(x, y), sampleRadius, .5f);
            divider += 1.0f;
        }
    }
    rays /= max(divider, 1.0f);
    //rays = RayTracing.Sample(defaultSampler, input.uv);
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

	float lengthToFragment = length(worldPos - CameraPosition);

//	float3 rayColor = lengthToFragment >= rays.a ? rays.rgb : float3(0, 0, 0);
	

	//return float4(rays.a, 0.0f, 0.0f, 1.0f);

    //return saturate(lerp(finalColor, rays, metallic.r) + ambient);
    float val = metallic.r >= 0.9f ? metallic.r : 0;
    return saturate(lerp(finalColor, rays, val) + ambient + specular);
	//return saturate(float4(rays.rgb, 1.0f));
}