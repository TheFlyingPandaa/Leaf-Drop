cbuffer RAY_BOX : register(b0)
{
    float4 ViewerPosition;
    uint4 Info; // X and Y are windowSize, Z and W are dimentions of threads
}

RWTexture2D<float4> outputTexture : register(u0);
StructuredBuffer<uint2> indices : register(t1);

[numthreads(32, 32, 1)]
void main(uint3 threadID : SV_GroupThreadID)
{
    if (threadID.x < Info.z * 32 && threadID.y < Info.w * 32)
    {
        uint2 threadIDOffset = indices[threadID.x + threadID.y * Info.z * 32];
        
        uint2 pixelLocation = threadID.xy + threadIDOffset * 32;

        float4 pixelMidPosition = float4(float2(pixelLocation) + float2(0.5f, 0.5f), 0.0f, 1.0f);

        float4 ray = normalize(pixelMidPosition - ViewerPosition);
      
        //outputTexture[index] = ray;
        outputTexture[pixelLocation] = float4(float2(threadIDOffset) / Info.zw, 0.0f, 1.0f);
    }

    //outputTexture[threadID.xy] = float4(1.0f, 1.0f, 0.0f, 1.0f);
}