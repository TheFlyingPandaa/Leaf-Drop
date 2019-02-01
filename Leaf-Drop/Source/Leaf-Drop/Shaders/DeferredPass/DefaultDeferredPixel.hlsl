SamplerState defaultSampler : register(s0);
Texture2D albedo : register(t0);


struct PS_INPUT
{
	float4 position : SV_Position;
	float2 uv : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	return albedo.Sample(defaultSampler, input.uv);
}