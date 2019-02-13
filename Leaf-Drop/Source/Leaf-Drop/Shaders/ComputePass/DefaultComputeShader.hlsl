cbuffer RAY_BOX : register(b0)
{
    float4 ViewerPosition;
    int4 Info; // X and Y are index. Z and W are windowSize
}

//RWTexture2D<float4> outputTexture : register(u0);
RWTexture2D<float4> outputTexture : register(u0);

[numthreads(32, 32, 1)]
void main( uint3 threadID : SV_DispatchThreadID )
{
    int2 index = threadID.xy + Info.xy * 32;
    float4 pixelPos = float4(float2(index) + float2(0.5f, 0.5f), 0.0f, 1.0f);
    float4 ray = normalize(pixelPos - ViewerPosition);

    //outputTexture[index.x + index.y * Info.w] = ray;
    outputTexture[index] = ray;
    
	return;
}