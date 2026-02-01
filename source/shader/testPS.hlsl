
#include "common.hlsl"

Texture2D BaseTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D OpacityTexture : register(t2);
StructuredBuffer<MATERIAL> MaterialArray : register(t3);

void main(in PS_IN In, out float4 outColor : SV_Target)
{
    outColor = float4(1, 1, 1, 1);
    outColor *= BaseTexture.Sample(SamplerState0, In.texCoord);
}
