
#include "common.hlsl"

Texture2D BaseTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D OpacityTexture : register(t2);
StructuredBuffer<MATERIAL> MaterialArray : register(t3);

void main(in PS_IN In, out float4 outColor : SV_Target0, out float4 outNormal : SV_Target1, out float4 outWorld : SV_Target2)
{
    outColor = In.color;
    outColor *= BaseTexture.Sample(SamplerState0, In.texCoord);
    outColor.a *= OpacityTexture.Sample(SamplerState0, In.texCoord).a;
    outColor *= MaterialArray[In.materialId].base;
    clip(outColor.a - 0.01f);
    outNormal = float4(normalize(In.normal), 1.0f);
    outWorld = float4(In.worldPosition.xyz, 1.0f);
    

}
