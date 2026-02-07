#include "common.hlsl"

[maxvertexcount(6)]
void main(in triangle PS_IN input[3], in uint Id : SV_PrimitiveID, inout TriangleStream<PS_IN> Out)
{
    for (int i = 0; i < 3; i++)
    {
        Out.Append(input[i]);
    }
    Out.RestartStrip();
    for (int i = 2; i >= 0; i--)
    {
        float3 offset = input[i].normal * 0.003f;
        float4 outlinePoistion = float4(input[i].worldPosition.xyz + offset, 1.0f);
        input[i].position = mul(outlinePoistion, ViewProjection);
        input[i].color *= float4(1.0f, 0.3f, 0.3f, 1.0f);
        Out.Append(input[i]);
    }
}