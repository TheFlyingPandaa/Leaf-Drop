struct LIGHT
{
	uint4 Type;
	float4 Position;
	float4 Color;
	float4 LightValues;
};

float4 LightCalculations(in LIGHT light, 
	in float4 cameraPosition, 
	in float4 worldPosition, 
	in float4 albedo, 
	in float4 normal, 
	in float4 metallic,
	inout float4 specular)
{
	float4 worldToCamera = normalize(cameraPosition - worldPosition);
	float4 posToLight = light.Position - worldPosition;
	float4 finalColor = float4(0, 0, 0, 0);
	float4 halfWayDir = 0;
	float distanceToLight = length(posToLight);
	float attenuation = 0;
	

	if (light.Type.x == 0) //pointlight
	{
		attenuation = light.LightValues.x / (1.0f + light.LightValues.y * pow(distanceToLight, light.LightValues.z));

		halfWayDir = normalize(posToLight + worldToCamera);

		specular += pow(max(dot(normal, halfWayDir), 0.0f), 128.0f) * length(metallic.rgb) * attenuation * light.Color;

		if (distanceToLight < light.LightValues.w)
		{
			finalColor = max(dot(normal, normalize(posToLight)), 0.0f) * light.Color * albedo * attenuation;
		}
	}
	else if (light.Type.x == 1) //Dir
	{
		finalColor = max(dot(normal, normalize(-float4(light.LightValues.xyz, 0))), 0.0f) * light.Color * albedo * light.LightValues.w;
	}
	return finalColor;
}