struct VS_INOUT
{
	float4 position : SV_Position;
	float2 uv : TEXCOORD;
};

VS_INOUT main(VS_INOUT vertex)
{
	return vertex;
}		   
