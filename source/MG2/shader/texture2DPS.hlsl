
#include "common.hlsl"

Texture2D BaseTexture : register(t0);
StructuredBuffer<MATERIAL> MaterialArray : register(t3);

void main(in PS_IN In, out float4 outColor : SV_Target)
{
    outColor = BaseTexture.Sample(SamplerState0, In.texCoord);
    outColor *= MaterialArray[In.materialId].base;
}
