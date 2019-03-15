#include "../LightCalculations.hlsli"

SamplerState defaultSampler : register(s0);

Texture2D textureAtlas[] : register(t0, space1);
StructuredBuffer<uint4> TextureIndex : register(t1);

cbuffer LightSize : register(b0, space2)
{
    uint4 LightValues;
}
StructuredBuffer<LIGHT> Lights : register(t0, space2);

cbuffer CAMERA : register(b0)
{
    float4x4 ViewProj;
    float4 CameraPosition;
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

    float4 albedo = textureAtlas[TextureIndex[0].x].Sample(defaultSampler, input.uv) * input.color;
    float4 normal = float4(normalize(input.normal.xyz + mul((2.0f * textureAtlas[TextureIndex[0].x + 1].Sample(defaultSampler, input.uv).xyz - 1.0f), input.TBN)), 0);
    float4 metallic = textureAtlas[TextureIndex[0].x + 2].Sample(defaultSampler, input.uv);

    output.albedo = albedo;
    output.normal = normal;
    output.metallic = metallic;

    float4 specular = float4(0, 0, 0, 1);
    float4 finalColor = float4(0, 0, 0, 1);

    for (uint i = 0; i < LightValues.x && i < 256; i++)
    {
        finalColor += LightCalculations(Lights[i],
			CameraPosition,
			input.worldPosition,
			albedo,
			normal,
			metallic,
			specular);
    }

    output.albedo = finalColor;

	return output;
}