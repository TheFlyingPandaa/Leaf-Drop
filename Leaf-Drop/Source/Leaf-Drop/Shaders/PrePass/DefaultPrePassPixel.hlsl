SamplerState defaultSampler : register(s0);

Texture2D textureAtlas[] : register(t0, space1);
StructuredBuffer<uint4> TextureIndex : register(t0);

struct PS_OUTPUT
{
    float4 World : SV_TARGET0;
    float4 Normal : SV_TARGET1;
    float4 Metallic : SV_TARGET2;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
    float4 worldPosition : WORLD;
    float4 normal : NORMAL;
    float3x3 TBN : TBN;
    float2 uv : UV;
};

PS_OUTPUT main(VS_OUTPUT input)
{
    PS_OUTPUT output = (PS_OUTPUT) 0;
    output.World = input.worldPosition;
    output.Normal = input.normal;
    output.Metallic = textureAtlas[TextureIndex[0].x + 2].Sample(defaultSampler, input.uv);
    return output;
}
