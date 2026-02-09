#include "common.hlsl"

[maxvertexcount(6)]
void main(in triangle PS_IN input[3], in uint Id : SV_PrimitiveID, inout TriangleStream<PS_IN> Out)
{
    [unroll]
    for (int i = 0; i < 3; i++)
    {
        Out.Append(input[i]);
    }
    
    Out.RestartStrip();
    
    [unroll]
    for (int j = 2; j >= 0; j--)
    {
        float3 offset = input[j].normal * 0.003f;
        float4 outlinePoistion = float4(input[j].worldPosition.xyz + offset, 1.0f);
        input[j].position = mul(outlinePoistion, ViewProjection);
        input[j].color *= float4(1.0f, 0.3f, 0.3f, 1.0f);
        Out.Append(input[j]);
    }
}