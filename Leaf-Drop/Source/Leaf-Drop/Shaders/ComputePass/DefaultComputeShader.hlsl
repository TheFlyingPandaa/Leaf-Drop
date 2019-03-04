#include "../LightCalculations.hlsli"

#define MAX_MIP 13
#define MIN_MIP 0
#define MIN_MIP_DIST 25
#define MAX_MIP_DIST 500

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

struct Triangle2
{
    Vertex v[3];
};

struct RAY_STRUCT
{
    float3 startPos;
    float3 normal;
    //uint2 pixelCoord;
    bool dispatch;
};


struct RayPayload
{
    float strength;
    float4 color;
};

struct TreeNode
{
    uint byteStart;

    float3 min;
    float3 max;

	uint nrOfChildren;
    uint ChildrenByteAddress[8];

	uint nrOfObjects;
};

struct AddressStack
{
    uint address;
    uint targetChildren;
};

struct MeshData
{
    float4x4 InverseWorld;
    float3 Min; // Local space
    float3 Max; //Local space
    uint MeshIndex; //Till Meshes[] 
    uint PADDING; // shit
};

cbuffer RAY_BOX : register(b0)
{
    float4 ViewerPosition; // World space
    uint4 Info; // X and Y are windowSize. Z is number of triangles
}

cbuffer LightSize : register(b0, space2)
{
    uint4 LightValues;
};

StructuredBuffer<LIGHT> Lights : register(t0, space2);

StructuredBuffer<MeshData> MeshDataBuffer : register(t0);
StructuredBuffer<RAY_STRUCT> RayStencil : register(t1);

StructuredBuffer<MeshData> MeshDataBuffer2 : register(t0, space3);
StructuredBuffer<Triangle2> Meshes[] : register(t1, space3);

RWTexture2D<float4> outputTexture : register(u0);
ByteAddressBuffer OcTreeBuffer : register(t0, space1);

SamplerState defaultTextureAtlasSampler : register(s0);
Texture2DArray TextureAtlas : register(t2);

TreeNode GetNode(in uint address, out uint meshIndexAdress)
{
    TreeNode node = (TreeNode)0;
    address += 4;
    node.byteStart = OcTreeBuffer.Load(address);
    address += 4;
    
    node.min = asfloat(OcTreeBuffer.Load3(address));
    address += 12;
    node.max = asfloat(OcTreeBuffer.Load3(address));
    address += 12 + 4;

    node.nrOfChildren = OcTreeBuffer.Load(address);
    
    address += 4 + 4 * 8;

    [unroll]
    for (uint j = 0; j < 8; j++)
    {
        node.ChildrenByteAddress[j] = OcTreeBuffer.Load(address);
        address += 4;
    }
    node.nrOfObjects = OcTreeBuffer.Load(address);
    address += 4;

    meshIndexAdress = address;

    return node;
}

void swap(inout float a, inout float b)
{
    float tmp = a;
    a = b;
    b = tmp;
}

MeshData GetMeshData(in uint address, in uint index)
{
    uint meshDataIndex = OcTreeBuffer.Load(address + index * 4);
    return MeshDataBuffer[meshDataIndex];
}

bool RayAABBFinalImprovement(in float3 bmin, in float3 bmax, in float3 ray, in float3 rayOrigin, inout float tmin)
{
    float tmax = -1;

    float3 invD = rcp(ray); // == 1.0f / ray
    float3 t0s = (bmin - rayOrigin) * invD;
    float3 t1s = (bmax - rayOrigin) * invD;
  
    float3 tsmaller = min(t0s, t1s);
    float3 tbigger = max(t0s, t1s);
  
    tmin = max(tmin, max(tsmaller[0], max(tsmaller[1], tsmaller[2])));
    tmax = min(tmax, min(tbigger[0], min(tbigger[1], tbigger[2])));

    return (tmin < tmax);
}

bool RayIntersectAABB(in float3 min, in float3 max, in float3 ray, in float3 rayOrigin, out float t)
{
    t = -1.0f;
    bool hit = true;

    float tmin = (min.x - rayOrigin.x) / ray.x;
    float tmax = (max.x - rayOrigin.x) / ray.x;
 
    if (tmin > tmax)
        swap(tmin, tmax);
 
    float tymin = (min.y - rayOrigin.y) / ray.y;
    float tymax = (max.y - rayOrigin.y) / ray.y;
 
    if (tymin > tymax)
        swap(tymin, tymax);
 

    if ((tmin > tymax) || (tymin > tmax)) 
        hit = false;
 
    if (hit)
    {
        if (tymin > tmin) 
            tmin = tymin;
 
        if (tymax < tmax) 
            tmax = tymax;
 
        float tzmin = (min.z - rayOrigin.z) / ray.z;
        float tzmax = (max.z - rayOrigin.z) / ray.z;
 
        if (tzmin > tzmax)
            swap(tzmin, tzmax);

        if ((tmin > tzmax) || (tzmin > tmax)) 
            hit = false;
        if (hit)
        {
            if (tzmin > tmin) 
                tmin = tzmin;
 
            if (tzmax < tmax) 
                tmax = tzmax;
 
            t = tmin;
        }
    }
 
    return hit;
}

bool RayIntersectTriangle(in Triangle2 tri, in float3 ray, in float3 rayOrigin, out float t, out float3 biCoord, out float3 intersectionPoint)
{
    const float EPSILON = 0.000001f;
    t = 9999.0f;
    biCoord.x = -1.0f;
    biCoord.y = -1.0f;
    biCoord.z = -1.0f;

    intersectionPoint = float3(0, 0, 0);

    float4 e1 = tri.v[1].pos - tri.v[0].pos;
    float4 e2 = tri.v[2].pos - tri.v[0].pos;

    float3 normal = cross(e1.xyz, e2.xyz);

    float3 h = cross(ray, e2.xyz);
    float a = dot(e1.xyz, h);

    if (a > -EPSILON && a < EPSILON) // If parallel with triangle
        return false;

    float f = 1.0f / a;
    float3 s = rayOrigin - tri.v[0].pos.xyz;
    float u = f * (dot(s, h));

    if (u < 0.0f || u > 1.0f)
        return false;

    float3 q = cross(s, e1.xyz);
    float v = f * dot(ray.xyz, q);

    if (v < 0.0 || u + v > 1.0f)
        return false;

    float tTemp = f * dot(e2.xyz, q);

    if (tTemp > 0.02f)
    {
        t = tTemp;
        biCoord.y = u;
        biCoord.z = v;
        biCoord.x = 1.0f - u - v;
        intersectionPoint = rayOrigin + ray * t;
        return true;
    }
    return false;
}

bool TraceTriangle(in float3 ray, in float3 origin, inout Triangle2 tri, out float3 biCoord, out float3 intersectionPoint)
{
    intersectionPoint = float3(0, 0, 0);
    biCoord = float3(1, 0, 0);
    tri = (Triangle2)0;

    AddressStack    nodeStack[8];
    uint            nodeStackSize = 0;
    uint            meshIndexAdress = 0;

    TreeNode node = GetNode(0, meshIndexAdress);

    bool triangleHit = false;
    float aabbT = 9999.0f;

    float triangleT = 9999.0f;
    float tempTriangleT = 9999.0f;
    float3 tempBi = float3(0, 0, 0);

    if (RayIntersectAABB(node.min, node.max, ray, origin, aabbT))
    {
        nodeStack[nodeStackSize].address = node.byteStart;
        nodeStack[nodeStackSize].targetChildren = 0;
        nodeStackSize++;
        while (nodeStackSize > 0)
        {
            uint currentNode = nodeStackSize - 1;
            node = GetNode(nodeStack[currentNode].address, meshIndexAdress);
            if (nodeStack[currentNode].targetChildren < node.nrOfChildren)
            {
                TreeNode child = GetNode(node.ChildrenByteAddress[nodeStack[currentNode].targetChildren++], meshIndexAdress);
                if (RayIntersectAABB(child.min, child.max, ray, origin, aabbT))
                {
                    if (child.nrOfObjects > 0)
                    {
                        for (uint objectIterator = 0; objectIterator < child.nrOfObjects; objectIterator++)
                        {
                            MeshData md = GetMeshData(meshIndexAdress, objectIterator);
                            float3 rayLocal = normalize(mul(float4(ray, 0.0f), md.InverseWorld)).xyz;
                            float3 originLocal = mul(float4(origin, 1.0f), md.InverseWorld).xyz;

                            if (RayIntersectAABB(md.Min, md.Max, rayLocal, originLocal, aabbT))
                            {
                                uint nrOfTriangles;
                                uint strides;
                                Meshes[md.MeshIndex].GetDimensions(nrOfTriangles, strides);
                                
                                for (uint triangleIndex = 0; triangleIndex < nrOfTriangles; triangleIndex++)
                                {
                                    if (RayIntersectTriangle(Meshes[md.MeshIndex][triangleIndex], rayLocal, originLocal, tempTriangleT, tempBi, intersectionPoint) && tempTriangleT < triangleT)
                                    {
                                        tri = Meshes[md.MeshIndex][triangleIndex];
                                        biCoord = tempBi;
                                        triangleT = tempTriangleT;
                                        triangleHit = true;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        nodeStack[nodeStackSize].address = child.byteStart;
                        nodeStack[nodeStackSize].targetChildren = 0;
                        nodeStackSize++;
                    }
                }
            }
            else
            {
                nodeStackSize--;
            }
        }
    }
    return triangleHit;
}

[numthreads(1, 1, 1)]
void main (uint3 threadID : SV_DispatchThreadID)
{
    float4 finalColor = float4(0, 0, 0, 1);
	
    uint rayStencilIndex = threadID.x + threadID.y * Info.x;
    
    uint2 pixelLocation =   uint2(threadID.xy);
    
    if (!RayStencil[rayStencilIndex].dispatch) return; // Can this be improved?

    float3 fragmentWorld =  RayStencil[rayStencilIndex].startPos;
    float3 fragmentNormal = RayStencil[rayStencilIndex].normal;

    float3 ray = normalize(fragmentWorld - ViewerPosition.xyz);
    //ray = normalize(ray - (2.0f * (fragmentNormal * (dot(ray, fragmentNormal)))));
    
    float3 intersectionPoint;
    Triangle2 tri;
    float3 uvw;

    float4 specular;

    float strenght = 1.0f;

    float distToCamera = length(fragmentWorld - ViewerPosition.xyz);

    for (uint rayBounce = 0; rayBounce < 1 && strenght > 0.0f; rayBounce++)
    {
        if (TraceTriangle(ray, ViewerPosition.xyz, tri, uvw, intersectionPoint))
        {
            outputTexture[pixelLocation] = float4(uvw, 1.0f);
            return;

            distToCamera += length(fragmentWorld - intersectionPoint);
            float mip = saturate((MIN_MIP_DIST - distToCamera) / (MIN_MIP_DIST - MAX_MIP_DIST));

            float finalMip = mip * (MAX_MIP - MIN_MIP);

            float2 uv = tri.v[0].uv * uvw.x + tri.v[1].uv * uvw.y + tri.v[2].uv * uvw.z;
     
            float4 albedo = TextureAtlas.SampleLevel(defaultTextureAtlasSampler, float3(uv, 0), finalMip);
            float4 normal = TextureAtlas.SampleLevel(defaultTextureAtlasSampler, float3(uv, 0 + 1), finalMip);
            float4 metall = TextureAtlas.SampleLevel(defaultTextureAtlasSampler, float3(uv, 0 + 2), finalMip);
            strenght -= 1.0f - metall.r;
        
            float4 ambient = float4(0.15f, 0.15f, 0.15f, 1.0f) * albedo;

            for (uint i = 0; i < LightValues.x; i++)
            {
                finalColor += LightCalculations(Lights[i], ViewerPosition, float4(intersectionPoint, 1), albedo, float4(normal.xyz, 0), metall, specular) * strenght;
            }
            finalColor += ambient;
            fragmentWorld = intersectionPoint;
            ray = normalize(ray - (2.0f * (fragmentNormal * (dot(ray, fragmentNormal)))));
        }
    }
    
    outputTexture[pixelLocation] = saturate(finalColor);
}