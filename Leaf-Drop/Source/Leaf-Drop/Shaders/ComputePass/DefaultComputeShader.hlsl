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
    node.byteSize = OcTreeBuffer.Load(address);
    address += 4;
    node.byteStart = OcTreeBuffer.Load(address);
    address += 4;


    node.position = asfloat(OcTreeBuffer.Load3(address));
    address += 12;
    node.axis = asfloat(OcTreeBuffer.Load3(address));
    address += 12;

    node.level = OcTreeBuffer.Load(address);
    address += 4;

    node.nrOfChildren = OcTreeBuffer.Load(address);
    address += 4;

	[unroll]
    for (uint i = 0; i < 8; i++)
    {
        node.ChildrenIndex[i] = OcTreeBuffer.Load(address);
        address += 4;
    }

    [unroll]
    for (uint j = 0; j < 8; j++)
    {
        node.ChildrenByteAddress[j] = OcTreeBuffer.Load(address);
        address += 4;
    }
    node.nrOfTris = OcTreeBuffer.Load(address);
    address += 4;

    tringlesAddress = address;

    node.min = node.position - node.axis;
    node.max = node.position + node.axis;

    return node;
}

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

bool RayIntersectTriangle(in Triangle tri, in float3 ray, in float3 rayOrigin, out float t, out float3 biCoord, out float3 intersectionPoint)
{
    const float EPSILON = 0.000001f;
    t = 9999.0f;
    biCoord.x = -1.0f;
    biCoord.y = -1.0f;
    biCoord.z = -1.0f;

    intersectionPoint = float3(0, 0, 0);

    float4 e1 = tri.v1.pos - tri.v0.pos;
    float4 e2 = tri.v2.pos - tri.v0.pos;

    float3 normal = cross(e1.xyz, e2.xyz);

    float3 h = cross(ray, e2.xyz);
    float a = dot(e1.xyz, h);

    if (a > -EPSILON && a < EPSILON) // If parallel with triangle
        return false;

    float f = 1.0f / a;
    float3 s = rayOrigin - tri.v0.pos.xyz;
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

struct AddressStack
{
    uint address;
    uint targetChildren;
};

struct LeafStack
{
    float t;
    uint triangleAddress;
    uint nrOfTriangles;
};

void SwapLeafStackElement(inout LeafStack e1, inout LeafStack e2)
{
    LeafStack tmp = e1;
    e1 = e2;
    e2 = tmp;
}

void SortLeafStack(in uint stackSize, inout LeafStack ls[256])
{
    bool swapped = true;

    for (uint i = 0; i < stackSize - 1 && swapped; i++)
    {
        swapped = false;

        for (uint j = 0; j < stackSize - i - 1; j++)
        {
            if (ls[j].t > ls[j + 1].t)
            {
                SwapLeafStackElement(ls[j], ls[j + 1]);
                swapped = true;
            }
        }
    }
}

bool TraceTriangle(in float3 ray, in float3 origin, inout Triangle tri, out float3 biCoord, out float3 intersectionPoint)
{
    intersectionPoint = float3(0, 0, 0);
    biCoord = float3(0, 0, 0);
    tri = (Triangle)0;

    AddressStack    nodeStack[256];
    uint            nodeStackSize = 0;

    LeafStack       leafStack[256];
    uint            leafStackSize = 0;

    uint            triangleAddress = 0;

    TreeNode        node = GetNode(0, triangleAddress);

    bool triangleHit = false;
    float aabbT = 9999.0f;

    uint counter = 0;

    if (RayIntersectAABB(node.min, node.max, ray, origin, aabbT))
    {
        nodeStack[nodeStackSize].address = node.byteStart;
        nodeStack[nodeStackSize].targetChildren = 0;
        nodeStackSize++;
        while (nodeStackSize > 0)
        {
            uint currentNode = nodeStackSize - 1;
            node = GetNode(nodeStack[currentNode].address, triangleAddress);
            if (nodeStack[currentNode].targetChildren < node.nrOfChildren)
            {
                TreeNode child = GetNode(node.ChildrenByteAddress[nodeStack[currentNode].targetChildren++], triangleAddress);
                if (RayIntersectAABB(child.min, child.max, ray, origin, aabbT))
                {
                    if (child.nrOfTris > 0)
                    {
                        leafStack[leafStackSize].t = aabbT;
                        leafStack[leafStackSize].nrOfTriangles = child.nrOfTris;
                        leafStack[leafStackSize].triangleAddress = triangleAddress;
                        leafStackSize++;
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

    
    //SortLeafStack(leafStackSize, leafStack);

    float triangleT = 9999.0f;
    float tempTriangleT = 9999.0f;
    float3 tempBi = float3(0, 0, 0);


    for (uint stackIterator = 0; stackIterator < leafStackSize; stackIterator++)
    {
        for (uint triIterator = 0; triIterator < leafStack[stackIterator].nrOfTriangles; triIterator++)
        {
            Triangle currentTri = GetTriangle(leafStack[stackIterator].triangleAddress, triIterator);
            if (RayIntersectTriangle(currentTri, ray, origin, tempTriangleT, tempBi, intersectionPoint) && tempTriangleT < triangleT)
            {
                tri = currentTri;
                biCoord = tempBi;
                triangleT = tempTriangleT;
                triangleHit = true;
            }

        }
    }

    return triangleHit;
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

    uint dummy;
    uint address = 0;
    
    if (TraceTriangle(rayWorld.xyz, startPosWorld.xyz, tri, uvw, intersectionPoint))
    {
        float2 uv = tri.v0.uv * uvw.x + tri.v1.uv * uvw.y + tri.v2.uv * uvw.z;
        float4 color = TextureAtlas.SampleLevel(defaultTextureAtlasSampler, float3(uv, 0), 0);
        outputTexture[pixelLocation] = float4(uvw,1);
    }
}