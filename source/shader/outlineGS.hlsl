#include "common.hlsl"

[maxvertexcount(6)]
void main(in triangle PS_IN input[3], in uint Id : SV_PrimitiveID, inout TriangleStream<PS_IN> Out)
{
    PS_IN o;
    for (int i = 0; i < 3; i++)
    {
        Out.Append(input[i]);
    }
    Out.RestartStrip();
    for (int i = 2; i >= 0; i--)
    {
        //o.position = input[i].position;
        //input[i].color = float4(1.0f, 0.0f, 0.0f, 1.0f);
        //input[i].worldPosition;
        //input[i].normal;
        
        
        float4 outlinePoistion = float4(input[i].worldPosition.xyz + input[i].normal * 0.003f, 1.0f);
        input[i].position = mul(outlinePoistion, ViewProjection);
        input[i].color *= float4(1.0f, 0.3f, 0.3f, 1.0f);
        Out.Append(input[i]);
        
        //float3 normal = normalize(input[i].normal);
        //normal = mul(float4(normal, 0.0f), ViewProjection).xyz;
        //float4 outlinePoistion = input[i].position + float4(normal * 0.005f, 0);
        //input[i].color = float4(0.0f, 0.0f, 0.0f, 1.0f);
        //input[i].position = outlinePoistion;
        //Out.Append(input[i]);
    }
}