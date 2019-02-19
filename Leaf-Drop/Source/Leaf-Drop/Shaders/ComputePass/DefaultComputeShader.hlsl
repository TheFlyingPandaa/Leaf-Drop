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

struct RAY_STRUCT
{
	float4	worldPos;
	uint2	pixelCoord;
};



RWTexture2D<float4> outputTexture : register(u0);
StructuredBuffer<Triangle> TriangleBuffer : register(t0);
StructuredBuffer<RAY_STRUCT> RayStencil : register(t1);

SamplerState	defaultTextureAtlasSampler : register(s0);
Texture2DArray	TextureAtlas : register(t2);

void BounceRay(in float4 ray, in float4 startPos, out float3 intersectionPoint, out Triangle tri, out bool hit, out float3 uvw, out uint index)
{
	const float EPSILON = 0.000001f;
	float minT = 9999.0f;
	index = -1;
	uvw.x = -1.0f;
	uvw.y = -1.0f;
	uvw.z = -1.0f;

	intersectionPoint = float3(0,0,0);
	tri = (Triangle)0;

	hit = false;

	for (uint i = 0; i < Info.z; i++)
	{
		Triangle tri = TriangleBuffer[i];

		float4 e1 = tri.v1.pos - tri.v0.pos;
		float4 e2 = tri.v2.pos - tri.v0.pos;

		float3 normal = cross(e1.xyz, e2.xyz);

		float3 h = cross(ray.xyz, e2.xyz);
		float a = dot(e1.xyz, h);

		if (a > -EPSILON && a < EPSILON) // If parallel with triangle
			continue;

		float f = 1.0f / a;
		float3 s = startPos.xyz - tri.v0.pos.xyz;
		float u = f * (dot(s, h));

		if (u < 0.0f || u > 1.0f)
			continue;

		float3 q = cross(s, e1.xyz);
		float v = f * dot(ray.xyz, q);

		if (v < 0.0 || u + v > 1.0f)
			continue;

		float t = f * dot(e2.xyz, q);

		if (t > 0.1f && t < minT)
		{
			minT = t;
			index = i;
			uvw.x = u;
			uvw.y = v;
			uvw.z = 1.0f - u - v;
		}

	}

	if (index != -1)
	{
		intersectionPoint = startPos.xyz + ray.xyz * minT;
		tri = TriangleBuffer[index];
		hit = true;
	}
}

[numthreads(1, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID)
{
    float4 finalColor = float4(0,0,0,1);
	
	uint2 pixelLocation = RayStencil[threadID.x].pixelCoord;
	float4 fragmentWorld = RayStencil[threadID.x].worldPos;

    float4 rayWorld = float4(normalize(fragmentWorld - ViewerPosition).xyz, 0.0f);

    float4 startPosWorld = ViewerPosition;

	float3 intersectionPoint;
	Triangle tri;
	bool hit = false;
	float3 uvw;
    uint index;

    BounceRay(rayWorld, startPosWorld, intersectionPoint, tri, hit, uvw, index);
	
	if (hit)
	{
		float4 e1 = tri.v1.pos - tri.v0.pos;
		float4 e2 = tri.v2.pos - tri.v0.pos;

		float3 normal = normalize(cross(e1.xyz, e2.xyz));
		float4 newRay = float4(normalize(rayWorld.xyz - (2.0f * (normal * (dot(rayWorld.xyz, normal))))), 0.0f);
		float4 newStartPos = float4(intersectionPoint, 1.0f);

        float2 uv;
        
        if (index % 2 == 0)
            uv = tri.v0.uv * uvw.z + tri.v1.uv * uvw.y + tri.v2.uv * uvw.x;
		else
            uv = tri.v0.uv * uvw.z + tri.v1.uv * uvw.x + tri.v2.uv * uvw.y;
	   //
       //float4 color = TextureAtlas.SampleLevel(defaultTextureAtlasSampler, float3(uv, 0), 0);
       //outputTexture[pixelLocation] = color;
	   //
       //return;

        BounceRay(newRay, newStartPos, intersectionPoint, tri, hit, uvw, index);

		if (hit)
		{
			float2 uv = tri.v0.uv * uvw.x + tri.v1.uv * uvw.y + tri.v2.uv * uvw.z;
			float4 color = TextureAtlas.SampleLevel(defaultTextureAtlasSampler, float3(uv, 0), 0);
			outputTexture[pixelLocation] = color;
        
		}

	}
}