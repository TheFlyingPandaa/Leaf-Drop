cbuffer RAY_BOX : register(b0)
{
	float4x4 ViewMatrixInverse;
	float4 ViewerPosition;
	uint4 Info; // X and Y are windowSize. Z is number of triangles
}

struct Vertex
{
	float4 pos;
	float4 normal;
	float2 uv;
};

struct Triangle
{
	Vertex v0, v1, v2;
};


RWTexture2D<float4> outputTexture : register(u0);
RWStructuredBuffer<uint2> indices : register(u1);
StructuredBuffer<Triangle> TriangleBuffer : register(t0);

[numthreads(32, 32, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    uint2 index = threadID.xy;
    if (index.x < Info.x && index.y < Info.y)
        outputTexture[index] = float4(0.0f, 0.0f, 0.0f, 0.0f);
}