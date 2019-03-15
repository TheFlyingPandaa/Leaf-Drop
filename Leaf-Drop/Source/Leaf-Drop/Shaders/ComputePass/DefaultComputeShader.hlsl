#include "../LightCalculations.hlsli"

#define RAY_BOUNCE 1

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
	float4 uv;
};

struct Triangle2
{
    Vertex v[3];
};

struct RAY_STRUCT
{
    float3 startPos;
    float3 normal;
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
    float4x4 World;
    float4x4 InverseWorld;
    float3 Min; // Local space
    float3 Max; //Local space
    uint MeshIndex; //Till Meshes[] 
    uint TextureIndex; //Texture index possible fence needed bobby please fix
};

cbuffer RAY_BOX : register(b0)
{
    float4 ViewerPosition; // World space
    uint4 Info; // X and Y are windowSize. Z is number of triangles
}



StructuredBuffer<float4> OffsetBuffer : register(t0, space5);

StructuredBuffer<LIGHT> Lights : register(t0, space2);

StructuredBuffer<MeshData> MeshDataBuffer : register(t0);
StructuredBuffer<RAY_STRUCT> RayStencil : register(t1);

StructuredBuffer<Triangle2> Meshes[] : register(t0, space3);

RWTexture2D<float4> outputTexture : register(u0);
ByteAddressBuffer OcTreeBuffer : register(t0, space1);

SamplerState defaultTextureAtlasSampler : register(s0);
Texture2D TextureAtlas[] : register(t0, space4);

TreeNode GetNode(in uint address, out uint meshIndexAdress)
{
    TreeNode node = (TreeNode)0;
    
    address += 4; // byteSize

    node.byteStart = OcTreeBuffer.Load(address);
    address += 4; // byteStart
    
    node.min = asfloat(OcTreeBuffer.Load3(address));
    address += 12; // min

    node.max = asfloat(OcTreeBuffer.Load3(address));
    address += 12; // max

    address += 4; // Level

    node.nrOfChildren = OcTreeBuffer.Load(address);
    address += 4; // nrOfChildren

    address += 4 * 8; // childrenIndices

    [unroll]
    for (uint j = 0; j < 8; j++)
    {
        node.ChildrenByteAddress[j] = OcTreeBuffer.Load(address);
        address += 4; // childrenByteAdress[i]
    }

    node.nrOfObjects = OcTreeBuffer.Load(address);
    address += 4; // numberOfObjects

    meshIndexAdress = address;

    return node;
}

void swap(inout float a, inout float b)
{
    float tmp = a;
    a = b;
    b = tmp;
}

Triangle2 WorldSpaceTriangle(in Triangle2 tri, in float4x4 worldMatrix)
{
    Triangle2 outTri = (Triangle2)0;

    [unroll]
    for (uint i = 0; i < 3; i++)
    {
        outTri.v[i].pos = mul(tri.v[i].pos, worldMatrix);
        outTri.v[i].normal = mul(tri.v[i].normal, worldMatrix);
        outTri.v[i].tangent = mul(tri.v[i].tangent, worldMatrix);
        outTri.v[i].bitangent = mul(tri.v[i].bitangent, worldMatrix);
        outTri.v[i].uv = tri.v[i].uv;
    }

    return outTri;
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
    const float EPSILON = 0.00001f;
    t = 9999.0f;
    biCoord.x = -1.0f;
    biCoord.y = -1.0f;
    biCoord.z = -1.0f;

    intersectionPoint = float3(0, 0, 0);

    float4 e1 = tri.v[1].pos - tri.v[0].pos;
    float4 e2 = tri.v[2].pos - tri.v[0].pos;

    float3 normal = cross(e1.xyz, e2.xyz);

    if (dot(normal, ray) >= 0.0f)
        return false;

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

    if (tTemp > EPSILON)
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

bool TraceTriangle(in float3 ray, in float3 origin, inout Triangle2 tri, out float3 biCoord, out float3 intersectionPoint, out float3 normal, inout uint textureIndex)
{
    intersectionPoint = float3(0, 0, 0);
    normal = float3(0, 0, 0);
    
    biCoord = float3(1, 0, 0);
    tri = (Triangle2)0;

    textureIndex = 0;

    float3 tempIntersectionPoint;
    

    AddressStack    nodeStack[8];
    uint            nodeStackSize = 0;
    uint            meshIndexAdress = 0;

    float4x4 worldMatrix = 0;

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
                                    Triangle2 boi = Meshes[md.MeshIndex][triangleIndex];                              
                                    if (RayIntersectTriangle(boi, rayLocal, originLocal, tempTriangleT, tempBi, tempIntersectionPoint) && tempTriangleT < triangleT)
                                    {
                                        intersectionPoint = tempIntersectionPoint;
                                        tri = boi;
                                        biCoord = tempBi;
                                        triangleT = tempTriangleT;
                                        triangleHit = true;
                                        worldMatrix = md.World;
                                        textureIndex = md.TextureIndex;

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

    if (triangleHit)
    {
        intersectionPoint = mul(float4(intersectionPoint, 1.0f), worldMatrix).xyz;
        normal = (tri.v[0].normal * biCoord.x + tri.v[1].normal * biCoord.y + tri.v[2].normal * biCoord.z).xyz;
        normal = normalize(mul(float4(normal, 0.0f), worldMatrix).xyz);
    }

    return triangleHit;
}

[numthreads(1, 1, 1)]
void main (uint3 threadID : SV_DispatchThreadID)
{    
    uint2 windowOffset = OffsetBuffer[0].xy * Info.xy;
    uint rayStencilIndex = (windowOffset.x + threadID.x) + (windowOffset.y + threadID.y) * Info.x;
    uint2 pixelLocation =   uint2(threadID.xy) + windowOffset;
    
    if (!RayStencil[rayStencilIndex].dispatch) return; // Can this be improved?
    
    float4 finalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

    float3 fragmentWorld =  RayStencil[rayStencilIndex].startPos;
    float3 fragmentNormal = RayStencil[rayStencilIndex].normal;

    outputTexture[pixelLocation] = float4(fragmentNormal.xyz, 1);
    return;
    float3 ray = normalize(fragmentWorld - ViewerPosition.xyz);
    ray = normalize(ray - (2.0f * (fragmentNormal * (dot(ray, fragmentNormal)))));
    
    uint textureIndex;
    float3 intersectionPoint;
    Triangle2 tri;
    float3 uvw;

    float4 specular;

    float strenght = 1.0f;

    float distToCamera = length(fragmentWorld - ViewerPosition.xyz);

    uint nrOfLights, dummy;
    Lights.GetDimensions(nrOfLights, dummy);
    
    for (uint rayBounce = 0; rayBounce < RAY_BOUNCE && strenght > 0.0f; rayBounce++)
    {
        float3 triNormal;

        //if (TraceTriangle(ray, ViewerPosition.xyz, tri, uvw, intersectionPoint, triNormal, textureIndex))
        if (TraceTriangle(ray, fragmentWorld + ray * 0.01f, tri, uvw, intersectionPoint, triNormal, textureIndex))
        {
            distToCamera += length(fragmentWorld - intersectionPoint);
            float mip = saturate((MIN_MIP_DIST - distToCamera) / (MIN_MIP_DIST - MAX_MIP_DIST));

            float finalMip = mip * (MAX_MIP - MIN_MIP);

            float2 uv = (tri.v[0].uv * uvw.x + tri.v[1].uv * uvw.y + tri.v[2].uv * uvw.z).xy;
     
            float4 albedo = TextureAtlas[textureIndex].SampleLevel(defaultTextureAtlasSampler, uv, 0);
            
            //float4 normal = TextureAtlas.SampleLevel(defaultTextureAtlasSampler, float3(uv, textureIndex + 1), finalMip);

            // We need TBN matrix
            float4 normal = float4(triNormal, 0.0f);
            
            float4 metall = TextureAtlas[textureIndex + 2].SampleLevel(defaultTextureAtlasSampler, uv, 0);

        
            float4 ambient = float4(0.15f, 0.15f, 0.15f, 1.0f) * albedo;

            float3 tmpColor = float3(0, 0, 0);
                       
            for (uint i = 0; i < 100; i++)
            {
                tmpColor += LightCalculations(Lights[i], ViewerPosition, float4(intersectionPoint, 1), albedo, float4(normal.xyz, 0), metall, specular).xyz;
            }

            tmpColor = saturate(tmpColor + ambient.rgb);
            
            finalColor = lerp(finalColor, float4(tmpColor, 1.0f), strenght);
            if (metall.r < 0.9f)
                strenght = 0;
            else
                strenght -= (1.0f - (metall.r));


            fragmentWorld = intersectionPoint;
            ray = normalize(ray - (2.0f * (fragmentNormal * (dot(ray, fragmentNormal)))));
        }
    }
    outputTexture[pixelLocation] = saturate(finalColor + specular);
}