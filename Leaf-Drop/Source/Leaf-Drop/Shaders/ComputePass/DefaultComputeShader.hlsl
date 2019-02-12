cbuffer RAY_BOX : register(b0)
{
    float4 ViewerPosition;
    int4 Index; // only X and Y are used
}

[numthreads(32, 32, 1)]
void main( uint3 threadID : SV_DispatchThreadID )
{
    float4 pixelPos = float4(threadID.xy + Index.xy, 0.0f, 1.0f);
    float4 ray = normalize(pixelPos - ViewerPosition);




	return;
}