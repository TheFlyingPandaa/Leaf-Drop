#include "../LightCalculations.hlsli"

cbuffer RAY_BOX : register(b0)
{
    float4 ViewerPosition; // World space
    uint4 Info; // X and Y are windowSize. Z is number of triangles
}

struct Vertex
{
	float4 pos;
	float4 normal;
    float4 tangent;
    float4 bitangent;
	float2 uv;
};

struct Triangle
{
	Vertex v0, v1, v2;
    uint textureIndexStart;
    
};

struct RAY_STRUCT
{
	float4	worldPos;
	uint2	pixelCoord;
};

struct RayPayload
{
    float strength;
    float4 color;
};

RWTexture2D<float4> outputTexture : register(u0);
StructuredBuffer<Triangle> TriangleBuffer : register(t0);
StructuredBuffer<RAY_STRUCT> RayStencil : register(t1);

SamplerState	defaultTextureAtlasSampler : register(s0);
Texture2DArray	TextureAtlas : register(t2);

StructuredBuffer<LIGHT> Lights : register(t0, space1);
cbuffer LightSize : register (b0, space1)
{
	uint4 LightValues;
}

void BounceRay2(in float4 rayDirection, in float4 startPos, inout RayPayload rayload, out float4 newDirectiom, out float4 newStartPos)
{
    const float EPSILON = 0.000001f;
    float minT = 9999.0f;
    int index = -1;
    float3 barycentrics = float3(-1, -1, -1);

    float3 intersectionPoint = float3(0, 0, 0);
    Triangle tri = (Triangle) 0;


    for (uint i = 0; i < Info.z; i++)
    {
        Triangle tri = TriangleBuffer[i];

        float4 e1 = tri.v1.pos - tri.v0.pos;
        float4 e2 = tri.v2.pos - tri.v0.pos;

        float3 normal = cross(e1.xyz, e2.xyz);

        float3 h = cross(rayDirection.xyz, e2.xyz);
        float a = dot(e1.xyz, h);

        if (a > -EPSILON && a < EPSILON) // If parallel with triangle
            continue;

        float f = 1.0f / a;
        float3 s = startPos.xyz - tri.v0.pos.xyz;
        float u = f * (dot(s, h));

        if (u < 0.0f || u > 1.0f)
            continue;

        float3 q = cross(s, e1.xyz);
        float v = f * dot(rayDirection.xyz, q);

        if (v < 0.0 || u + v > 1.0f)
            continue;

        float t = f * dot(e2.xyz, q);

        if (t > 0.1f && t < minT)
        {
            minT = t;
            index = i;
            barycentrics.x = 1.0f - u - v;
            barycentrics.y = u;
            barycentrics.z = v;
        }

    }

    if (index != -1)
    {
        intersectionPoint = startPos.xyz + rayDirection.xyz * minT;
        tri = TriangleBuffer[index];

        float2 uv = tri.v0.uv * barycentrics.x + tri.v1.uv * barycentrics.y + tri.v2.uv * barycentrics.z;

        //normal mapping bobby please fix
        float4 albedo = TextureAtlas.SampleLevel(defaultTextureAtlasSampler, float3(uv, tri.textureIndexStart), 0);
        //float4 normal = TextureAtlas.SampleLevel(defaultTextureAtlasSampler, float3(uv, tri.textureIndexStart + 1), 0);
        float4 metall = TextureAtlas.SampleLevel(defaultTextureAtlasSampler, float3(uv, tri.textureIndexStart + 2), 0);


        rayload.strength -= 1.0f - metall.r;

        float4 e1 = tri.v1.pos - tri.v0.pos;
        float4 e2 = tri.v2.pos - tri.v0.pos;

        float3 normal = normalize(cross(e1.xyz, e2.xyz));
        newDirectiom = float4(normalize(rayDirection.xyz - (2.0f * (normal * (dot(rayDirection.xyz, normal))))), 0.0f);
        newStartPos = float4(intersectionPoint, 1.0f);
        
		float4 specular;
        float4 finalColor = float4(0, 0, 0, 0);
        for (uint i = 0; i < LightValues.x; i++)
        {
            finalColor += LightCalculations(Lights[i], ViewerPosition, float4(intersectionPoint, 1), albedo, float4(normal, 0), metall, specular);
        }
        rayload.color = finalColor * rayload.strength;

    }
}

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
			uvw.x = 1.0f - u - v;
			uvw.y = u;
			uvw.z = v;
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

    RayPayload rayLoad = (RayPayload)0;

	/*
	Test
	*/
	rayLoad.strength = 1.0f;
	BounceRay2(rayWorld, startPosWorld, rayLoad, rayWorld, startPosWorld);
	rayLoad.color = float4(0, 0, 0, 0);
	/*
	End Test
	*/


	rayLoad.strength = 1.0f;
    for (uint i = 0; i < 3; i++)
    {
        if (rayLoad.strength > 0)
            BounceRay2(rayWorld, startPosWorld, rayLoad, rayWorld, startPosWorld);
    }
    
    outputTexture[pixelLocation] = rayLoad.color;
}