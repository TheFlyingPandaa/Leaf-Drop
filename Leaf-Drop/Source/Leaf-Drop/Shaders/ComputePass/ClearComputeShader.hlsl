cbuffer RAY_BOX : register(b0)
{
	float4 ViewerPosition;
	int4 Info; // X and Y windowSize
}

RWTexture2D<float4> outputTexture : register(u0);

StructuredBuffer<uint2> indices : register(t1);

[numthreads(32, 32, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    uint2 index = threadID.xy;
    if (index.x < Info.x && index.y < Info.y)
        outputTexture[index] = float4(0.0f, 0.0f, 0.0f, 0.0f);
}