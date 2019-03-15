#include "../LightCalculations.hlsli"
SamplerState defaultSampler : register(s0);

Texture2D Position		: register(t0, space0);
Texture2D Specular		: register(t0, space1);
Texture2D Albedo		: register(t0, space2);
Texture2D Metallic		: register(t0, space3);
Texture2D RayTracing	: register(t0, space4);



cbuffer Camera : register (b0)
{
	float4 CameraDirection;
	float4 CameraPosition;
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
    float4 specular = Specular.Sample(defaultSampler, input.uv);
	float4 albedo	= Albedo.Sample(defaultSampler, input.uv);
	float4 metallic = Metallic.Sample(defaultSampler, input.uv);

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
            float4 rayColor = RayTracing.Sample(defaultSampler, smTex) * multiplier(int2(x, y), sampleRadius, 4.0f * metallic.r);
            rays += rayColor;
            if (length(rayColor.rgb) > 0)
                divider += 1.0f;
        }
    }
    rays /= max(divider, 1.0f);
    float4 ambient = float4(0.15f, 0.15f, 0.15f, 1.0f) * albedo;


    float val = metallic.r >= 0.9f ? metallic.r : 0;
    return saturate(lerp(albedo, rays, val) + ambient + specular);
}