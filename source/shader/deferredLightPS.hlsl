
#include "common.hlsl"

Texture2D ColorTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D WorldPositionTexture : register(t2);
Texture2D DirectionalShadowMapTexture : register(t3);

void main(in PS_IN In, out float4 outColor : SV_Target)
{
    outColor = ColorTexture.Sample(SamplerState0, In.texCoord);

    float3 light = Ambient.rgb;
    
    float3 normal = NormalTexture.Sample(SamplerState0, In.texCoord).xyz;
    normal = normalize(normal);
    
    float d = dot(normal, -normalize(DirectLightDirection.xyz));
    float3 directLight = (d > 0.5f) ? DirectLightColor.rgb : DirectLightColor.rgb * pow(saturate(d / 0.5f), 0.5);
    
    //float3 worldPosition = WorldPositionTexture.Sample(SamplerState0, In.texCoord).xyz;
    //float4 shadowMapPosition = mul(float4(worldPosition, 1.0f), DirectionalShadowViewProjection);
    //float3 ndc = shadowMapPosition.xyz / shadowMapPosition.w;
    //float2 shadowUV = float2(ndc.x * 0.5f + 0.5f, -ndc.y * 0.5f + 0.5f);
    //if (shadowUV.x <= 1.0f && shadowUV.x >= 0.0f &&
    //    shadowUV.y <= 1.0f && shadowUV.y >= 0.0f)
    //{
    //    float z = DirectionalShadowMapTexture.Sample(SamplerState0, shadowUV).r;
    //    if (shadowMapPosition.z - 0.002f > z)
    //        directLight *= 0.5f;

    //}
    
    float3 upper = float3(0.0f, 0.0f, 1.0f);
    float3 right = cross(DirectLightDirection.xyz, upper);
    right *= 0.02f;
    upper *= 0.02f;
    float3 offsets[] = 
    {
        float3(0.0f, 0.0f, 0.0f),
         upper - right,
         upper + right,
        -upper - right,
        -upper + right,
        ( upper - right) * 2,
        ( upper + right) * 2,
        (-upper - right) * 2,
        (-upper + right) * 2
    };
    float f = 1.0f;
    [unroll]
    for (uint i = 0; i < 9; i++)
    {
        float3 worldPosition = WorldPositionTexture.Sample(SamplerState0, In.texCoord).xyz + offsets[i];
        float4 shadowMapPosition = mul(float4(worldPosition, 1.0f), DirectionalShadowViewProjection);
        float3 ndc = shadowMapPosition.xyz / shadowMapPosition.w;
        float2 shadowUV = float2(ndc.x * 0.5f + 0.5f, -ndc.y * 0.5f + 0.5f);
        if (shadowUV.x < 0.98f && shadowUV.x > 0.02f &&
            shadowUV.y < 0.98f && shadowUV.y > 0.02f)
        {
            float z = DirectionalShadowMapTexture.Sample(SamplerState0, shadowUV).r;
            if (shadowMapPosition.z - 0.002f > z)
                directLight *= 0.9f;
        }
    }
    
    
    light += directLight * f;
    
    // Toon
    //float3 cameraV = normalize(worldPosition - CameraPosition);
    //light *= (abs(dot(cameraV, normal)) < 0.4f) ? 0.1f : 1.0f;
    //light *= (dot(-cameraV, normal) < 0.05f) ? 0.1f : 1.0f;
    
        outColor.rgb *= light;

}
