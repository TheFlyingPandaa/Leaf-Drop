cbuffer RAY_BOX : register(b0)
{
	float4 ViewerPosition;
	int4 Info; // X and Y are index. Z and W are windowSize
}

RWTexture2D<float4> outputTexture : register(u0);

[numthreads(32, 32, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint startIndexX = DTid.x * (Info.z / 32u);
	uint endIndexX = startIndexX + (Info.z / 32u);
	uint startIndexY = DTid.y * (Info.w / 32u);
	uint endIndexY = startIndexY + (Info.w / 32u);

	for (uint i = startIndexX; i < endIndexX; i++)
	{
		for (uint j = startIndexY; j < endIndexY; j++)
		{
			outputTexture[int2(i,j)] = float4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	}

	/*for (uint i = 0; i < 32; i++)
	{
		for (uint j = 0; j < 32; j++)
		{
			outputTexture[int2(i, j)] = float4(1.0f, 0.0f, 1.0f, 0.0f);
		}
	}*/

}