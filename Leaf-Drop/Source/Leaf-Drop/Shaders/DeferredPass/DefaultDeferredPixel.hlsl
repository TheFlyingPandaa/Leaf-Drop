SamplerState defaultSampler : register(s0);

Texture2D Position	: register(t0, space0);
Texture2D Normal    : register(t0, space1);
Texture2D Albedo    : register(t0, space2);
Texture2D Metallic  : register(t0, space3);


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

	return worldPos;
}