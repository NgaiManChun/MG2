
#include "common.hlsl"

void main(uint vertexId : SV_VertexID, out PS_IN Out)
{
    float4 vertexes[4] =
    {
        { -1.0f, 1.0f, 0.1f, 1.0f },
        { 1.0f, 1.0f, 0.1f, 1.0f },
        { -1.0f, -1.0f, 0.1f, 1.0f },
        { 1.0f, -1.0f, 0.1f, 1.0f }
    };
    
    float2 texCoord[4] =
    {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 0.0f, 1.0f },
        { 1.0f, 1.0f }
    };
    
    Out.position = vertexes[vertexId];
    Out.texCoord = texCoord[vertexId];

}