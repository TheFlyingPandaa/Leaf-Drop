cbuffer RAY_BOX : register(b0)
{
    float4 ViewerPosition; // World space
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
struct RAY_STRUCT
{
	float4	worldPos;
	uint2	pixelCoord;
};
StructuredBuffer<RAY_STRUCT> RayStencil : register(t1);
StructuredBuffer<Triangle> TriangleBuffer : register(t0);



[numthreads(1, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
	const float EPSILON = 0.000001f;
	
    float4 finalColor = float4(1,0,0,1);
	
	uint2 pixelLocation = RayStencil[threadID.x].pixelCoord;
	float4 fragmentWorld = RayStencil[threadID.x].worldPos;

    float4 rayWorld = float4(normalize(fragmentWorld - ViewerPosition).xyz, 0.0f);

    float4 startPosWorld = ViewerPosition;

	float minT = 999.0f;
	int index = -1;

	for (uint i = 0; i < Info.z; i++)
	{
		Triangle tri = TriangleBuffer[i];

		float4 e1 = tri.v1.pos - tri.v0.pos;
		float4 e2 = tri.v2.pos - tri.v0.pos;

		float3 h = cross(rayWorld.xyz, e2.xyz);
		float a = dot(e1.xyz, h);

		if (a > -EPSILON && a < EPSILON)
			continue;
		float f = 1.0f / a;
		float3 s = startPosWorld.xyz - tri.v0.pos.xyz;
		float u = f * (dot(s, h));

		if (u < 0.0f || u > 1.0f)
			continue;
		
		float3 q = cross(s, e1.xyz);
		float v = f * dot(rayWorld.xyz, q);

		if (v < 0.0 || u + v > 1.0f)
			continue;
		
		float t = f * dot(e2.xyz, q);

		if (t > EPSILON && t < minT)
		{
			minT = t;
			index = i;
        }
		
	}

	if (index != -1)
	{
		float3 intersctionPoint = startPosWorld.xyz + rayWorld.xyz * minT;
		finalColor = float4(1,1,1, length(intersctionPoint - startPosWorld.xyz));
	}
	
	outputTexture[pixelLocation] = finalColor;
}