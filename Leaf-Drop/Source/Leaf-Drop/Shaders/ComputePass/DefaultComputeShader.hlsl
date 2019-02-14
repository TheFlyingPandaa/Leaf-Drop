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
	Vertex v1, v2, v3;
};


StructuredBuffer<Triangle> TriangleBuffer : register(t1);

RWTexture2D<float4> outputTexture : register(u0);
RWStructuredBuffer<uint2> indices : register(u1);

[numthreads(1, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
	uint2 pixelLocation = indices[threadID.x];
	float4 pixelMidPosition = float4(float2(pixelLocation) + float2(0.5f, 0.5f), 0.0f, 1.0f);
	float4 rayViewPosition = pixelMidPosition - ViewerPosition;

	float4 rayWorld = float4(normalize(mul(rayViewPosition, ViewMatrixInverse)).xyz, 0.0f);
	float4 startPosWorld = float4(mul(ViewerPosition, ViewMatrixInverse).xyz, 1.0f);



	outputTexture[indices[threadID.x]] = rayWorld + startPosWorld;
}