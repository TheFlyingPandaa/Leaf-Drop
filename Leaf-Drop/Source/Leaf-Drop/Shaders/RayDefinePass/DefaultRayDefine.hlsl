#define REFLECTION_VAL 0.9f

SamplerState defaultSampler : register(s0);
SamplerComparisonState compSampler : register(s1);

Texture2D WorldPosition : register(t0, space1);
Texture2D Normal        : register(t1, space1);
Texture2D Metallic      : register(t2, space1);

struct RAY_STRUCT
{
    float3 startPos;
    float3 normal;
    bool dispatch;
};

RWStructuredBuffer<RAY_STRUCT> RayStencil : register(u0);

struct PS_INPUT
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

void main(PS_INPUT input)
{
    float metallic = Metallic.Sample(defaultSampler, input.uv).r;
    float3 pos = WorldPosition.Sample(defaultSampler, input.uv).xyz;
    float3 normal = Normal.Sample(defaultSampler, input.uv).xyz;

    //uint2 rayStencilIndex = uint2(input.position.xy / RAY_DIV);
    uint accessIndex = (uint) input.position.x + (uint) input.position.y * RAY_WIDTH;

    RAY_STRUCT rs;
    rs.startPos = pos;
    rs.normal = normal;
    rs.dispatch = metallic >= REFLECTION_VAL;

    RayStencil[accessIndex] = rs;
}