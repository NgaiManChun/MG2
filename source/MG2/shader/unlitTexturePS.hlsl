
#include "common.hlsl"

Texture2D BaseTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D OpacityTexture : register(t2);
StructuredBuffer<MATERIAL> MaterialArray : register(t3);

void main(in PS_IN In, out float4 outColor : SV_Target0, out float4 outNormal : SV_Target1, out float4 outWorld : SV_Target2)
{
    MATERIAL material = MaterialArray[In.materialId];
    outColor = In.color;
    outColor *= material.base;
    
    if (material.baseTextureId != 0xffffffff)
    {
        outColor *= BaseTexture.Sample(SamplerState0, In.texCoord);
    }
    
    if (material.opacityTextureId != 0xffffffff)
    {
        outColor.a *= OpacityTexture.Sample(SamplerState0, In.texCoord).a;
    }
    clip(outColor.a - 0.01f);
    
    outNormal = float4(normalize(In.normal), 1.0f);
    
    outWorld = In.worldPosition;
    

}
