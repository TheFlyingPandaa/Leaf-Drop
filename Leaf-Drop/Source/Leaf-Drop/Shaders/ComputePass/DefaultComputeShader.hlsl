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

struct TreeNode
{
	uint byteSize;
    uint byteStart;
	
	float3 position;
	float3 axis;

    float3 min;
    float3 max;

	uint level;
	uint nrOfChildren;
	uint ChildrenIndex[8];
    uint ChildrenByteAddress[8];

	uint nrOfTris;
};

RWTexture2D<float4> outputTexture : register(u0);

StructuredBuffer<Triangle> TriangleBuffer : register(t0);
ByteAddressBuffer OcTreeBuffer : register(t0, space1);

StructuredBuffer<RAY_STRUCT> RayStencil : register(t1);

SamplerState defaultTextureAtlasSampler : register(s0);
Texture2DArray TextureAtlas : register(t2);

TreeNode GetNode(in uint address, out uint tringlesAddress)
{
    TreeNode node = (TreeNode)0;
    node.byteSize = OcTreeBuffer.Load(address += 4);
    node.byteStart = OcTreeBuffer.Load(address += 4);

    node.position = asfloat(OcTreeBuffer.Load3(address += 12));
    node.axis = asfloat(OcTreeBuffer.Load3(address += 12));

    node.level = OcTreeBuffer.Load(address += 4);
    node.nrOfChildren = OcTreeBuffer.Load(address += 4);

	[unroll]
    for (uint i = 0; i < 8; i++)
    {
        node.ChildrenIndex[i] = OcTreeBuffer.Load(address += 4);
    }
    [unroll]
    for (uint j = 0; j < 8; j++)
    {
        node.ChildrenByteAddress[j] = OcTreeBuffer.Load(address += 4);
    }
    node.nrOfTris = OcTreeBuffer.Load(address += 4);
    tringlesAddress = address;

    node.min = node.position - node.axis;
    node.max = node.position + node.axis;

    return node;
}

struct Stack
{
    uint address;
    uint parentIndex;
    uint currentChild;
};

void swap(inout float a, inout float b)
{
    float tmp = a;
    a = b;
    b = tmp;
}

Triangle GetTriangle(in uint address, in uint index)
{
    uint triIndex = OcTreeBuffer.Load(address + index * 4);
    return TriangleBuffer[triIndex];
}

//bool RayAABBFinalImprovement(in float3 ray, in float3 origin, in float3 min, in float3 max, out float tmin, out float tmax)
//{
//    float3 invD = rcp(ray); // == 1.0f / ray
//    float3 t0s = (min - origin) * invD;
//    float3 t1s = (max - origin) * invD;
    
//    float3 tsmaller = min(t0s, t1s);
//    float3 tbigger = max(t0s, t1s);
    
//    tmin = max(tmin, max(tsmaller[0], max(tsmaller[1], tsmaller[2])));
//    tmax = min(tmax, min(tbigger[0], min(tbigger[1], tbigger[2])));

//    return (tmin < tmax);
//}

bool RayIntersectAABB(in float3 min, in float3 max, in float3 ray, in float3 rayOrigin, out float t)
{
    t = -1.0f;

    float tmin = (min.x - rayOrigin.x) / ray.x;
    float tmax = (max.x - rayOrigin.x) / ray.x;
 
    if (tmin > tmax)
        swap(tmin, tmax);
 
    float tymin = (min.y - rayOrigin.y) / ray.y;
    float tymax = (max.y - rayOrigin.y) / ray.y;
 
    if (tymin > tymax)
        swap(tymin, tymax);
 

    
    if ((tmin > tymax) || (tymin > tmax)) 
        return false;
 
    if (tymin > tmin) 
        tmin = tymin;
 
    if (tymax < tmax) 
        tmax = tymax;
 
    float tzmin = (min.z - rayOrigin.z) / ray.z;
    float tzmax = (max.z - rayOrigin.z) / ray.z;
 
    if (tzmin > tzmax)
        swap(tzmin, tzmax);
 
    if ((tmin > tzmax) || (tzmin > tmax)) 
        return false;
 
    if (tzmin > tmin) 
        tmin = tzmin;
 
    if (tzmax < tmax) 
        tmax = tzmax;
 
    t = tmin;

    return true;
}
bool RayIntersectTriangle(in Triangle tri, in float3 ray, in float3 rayOrigin, out float t, out float3 biCoord)
{
    const float EPSILON = 0.000001f;
    float minT = 9999.0f;
    t = -1.0f;
    biCoord.x = -1.0f;
    biCoord.y = -1.0f;
    biCoord.z = -1.0f;

    float4 e1 = tri.v1.pos - tri.v0.pos;
    float4 e2 = tri.v2.pos - tri.v0.pos;

    float3 normal = cross(e1.xyz, e2.xyz);

    float3 h = cross(ray.xyz, e2.xyz);
    float a = dot(e1.xyz, h);

    if (a > -EPSILON && a < EPSILON) // If parallel with triangle
        return false;

    float f = 1.0f / a;
    float3 s = rayOrigin.xyz - tri.v0.pos.xyz;
    float u = f * (dot(s, h));

    if (u < 0.0f || u > 1.0f)
        return false;

    float3 q = cross(s, e1.xyz);
    float v = f * dot(ray.xyz, q);

    if (v < 0.0 || u + v > 1.0f)
        return false;

    t = f * dot(e2.xyz, q);

    if (t > 0.1f && t < minT)
    {
        t = minT;
        biCoord.x = 1.0f - u - v;
        biCoord.y = u;
        biCoord.z = v;
        return true;
    }
    return false;
}

void BounceRay (in float4 ray, in float4 startPos, out float3 intersectionPoint, out Triangle tri, out bool hit, out float3 uvw, out uint index)
{
    const float EPSILON = 0.000001f;
    float minT = 9999.0f;
    index = -1;
    uvw.x = -1.0f;
    uvw.y = -1.0f;
    uvw.z = -1.0f;

    intersectionPoint = float3(0, 0, 0);
    tri = (Triangle) 0;

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
            uvw.y = u;
            uvw.z = v;
            uvw.x = 1.0f - u - v;
        }

    }

    if (index != -1)
    {
        intersectionPoint = startPos.xyz + ray.xyz * minT;
        tri = TriangleBuffer[index];
        hit = true;
    }
}

bool GetClosestTriangle(in float3 ray, in float3 origin, inout Triangle tri, inout float3 biCoord)
{
    float tClosest = 9999.0f;
    bool triHit = false;

    TreeNode node;
    uint triangleAddress; //lol
    uint nodeAddress = 0;
    node = GetNode(nodeAddress, triangleAddress);

    uint stackSize = 0;
    
    Stack addressStack[256];
    addressStack[stackSize].address = nodeAddress;
    addressStack[stackSize].currentChild = 0;
    addressStack[stackSize++].parentIndex = 0;

    float t = -1;
    while (stackSize > 0)
    {
        uint currentParent = stackSize - 1;
        node = GetNode(addressStack[stackSize - 1].address, triangleAddress);
        if (RayIntersectAABB(node.min, node.max, ray, origin, t))
        {
            bool hit = false;
            if (node.nrOfChildren > 0)
            {
                //Add HitChildren
                for (uint i = 0; i < node.nrOfChildren; i++)
                {
                    uint dummy;
                    TreeNode child = GetNode(node.ChildrenByteAddress[i], dummy);
                    if (RayIntersectAABB(child.min, child.max, ray, origin, t))
                    {
                        addressStack[stackSize].address = node.ChildrenByteAddress[i];
                        addressStack[stackSize++].parentIndex = currentParent;
                        hit = true;
                    }
                }
            }
            else if (node.nrOfTris > 0)
            {
                
                for (uint i = 0; i < node.nrOfTris; i++)
                {
                    Triangle current = GetTriangle(triangleAddress, i);
                    float t;
                    if (RayIntersectTriangle(current, ray, origin, t, biCoord) && t < tClosest)
                    {
                        tri = current;
                        tClosest = t;
                        triHit = true;

                    }
                }
            }
            if (!hit)
            {
                stackSize = addressStack[stackSize - 1].parentIndex - 1;
            }
        }
    }

    return triHit;
}

[numthreads(1, 1, 1)]

void main (uint3 threadID : SV_DispatchThreadID)
{
    float4 finalColor = float4(0, 0, 0, 1);
	
    uint2 pixelLocation = RayStencil[threadID.x].pixelCoord;
    float4 fragmentWorld = RayStencil[threadID.x].worldPos;

    float4 rayWorld = float4(normalize(fragmentWorld - ViewerPosition).xyz, 0.0f);

    float4 startPosWorld = ViewerPosition;

    float3 intersectionPoint;
    Triangle tri;
    bool hit = false;
    float3 uvw;

    if (GetClosestTriangle(rayWorld.xyz, startPosWorld.xyz, tri, uvw))
    {
        float2 uv = tri.v0.uv * uvw.x + tri.v1.uv * uvw.y + tri.v2.uv * uvw.z;
        float4 color = TextureAtlas.SampleLevel(defaultTextureAtlasSampler, float3(uv, 0), 0);
        outputTexture[pixelLocation] = color;
    }


 //   BounceRay(rayWorld, startPosWorld, intersectionPoint, tri, hit, uvw, index);
	
 //   if (hit)
 //   {
 //       float4 e1 = tri.v1.pos - tri.v0.pos;
 //       float4 e2 = tri.v2.pos - tri.v0.pos;

 //       float3 normal = normalize(cross(e1.xyz, e2.xyz));
 //       float4 newRay = float4(normalize(rayWorld.xyz - (2.0f * (normal * (dot(rayWorld.xyz, normal))))), 0.0f);
 //       float4 newStartPos = float4(intersectionPoint, 1.0f);

 //       float2 uv;
        
 //       if (index % 2 == 0)
 //           uv = tri.v0.uv * uvw.z + tri.v1.uv * uvw.y + tri.v2.uv * uvw.x;
 //       else
 //           uv = tri.v0.uv * uvw.z + tri.v1.uv * uvw.x + tri.v2.uv * uvw.y;
	////
 //   //float4 color = TextureAtlas.SampleLevel(defaultTextureAtlasSampler, float3(uv, 0), 0);
 //   //outputTexture[pixelLocation] = color;
	////
 //   //return;

 //       BounceRay(newRay, newStartPos, intersectionPoint, tri, hit, uvw, index);

 //       if (hit)
 //       {
 //           float2 uv = tri.v0.uv * uvw.x + tri.v1.uv * uvw.y + tri.v2.uv * uvw.z;
 //           float4 color = TextureAtlas.SampleLevel(defaultTextureAtlasSampler, float3(uv, 0), 0);
 //           outputTexture[pixelLocation] = color;
        
 //       }

 //   }
}