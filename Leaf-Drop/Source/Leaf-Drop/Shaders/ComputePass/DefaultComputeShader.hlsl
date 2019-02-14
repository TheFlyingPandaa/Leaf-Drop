cbuffer RAY_BOX : register(b0)
{
    float4 ViewerPosition;
    uint4 Info; // X and Y are windowSize
}

RWTexture2D<float4> outputTexture : register(u0);
RWStructuredBuffer<uint2> indices : register(u1);

[numthreads(1, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
	outputTexture[indices[threadID.x]] = float4(1.0f, 1.0f, 0.0f, 1.0f);
}