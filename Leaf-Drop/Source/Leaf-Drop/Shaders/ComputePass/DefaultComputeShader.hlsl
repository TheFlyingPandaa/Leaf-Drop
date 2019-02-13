cbuffer RAY_BOX : register(b0)
{
    float4 ViewerPosition;
    int4 Index; // only X and Y are used
}

//RWTexture2D<float4> outputTexture : register(t0);

[numthreads(32, 32, 1)]
void main( uint3 threadID : SV_DispatchThreadID )
{
    int2 index = threadID.xy + Index.xy;
    float4 pixelPos = float4(float2(index) + float2(0.5f, 0.5f), 0.0f, 1.0f);
    float4 ray = normalize(pixelPos - ViewerPosition);

    //outputTexture[index] = ray;
    
	return;
}