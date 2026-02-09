
#include "common.hlsl"

void main(uint vertexId : SV_VertexID, out PS_IN Out)
{
    static float4 vertexes[4] =
    {
        { -1.0f, 1.0f, 0.1f, 1.0f },
        { 1.0f, 1.0f, 0.1f, 1.0f },
        { -1.0f, -1.0f, 0.1f, 1.0f },
        { 1.0f, -1.0f, 0.1f, 1.0f }
    };
    
    static float2 texCoord[4] =
    {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 0.0f, 1.0f },
        { 1.0f, 1.0f }
    };
    
    Out.position = vertexes[vertexId];
    Out.texCoord = texCoord[vertexId];
    
    Out.worldPosition = float4(0.0f, 0.0f, 0.0f, 0.0f);
    Out.normal = float3(0.0f, 0.0f, -1.0f);
    Out.tangent = float3(0.0f, 1.0f, 0.0f);
    Out.bitangent = float3(1.0f, 0.0f, 0.0f);
    Out.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    Out.materialId = MaterialId;

}