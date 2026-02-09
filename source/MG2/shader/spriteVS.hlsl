
#include "common.hlsl"

StructuredBuffer<float4x4> DynamicMartixArray : register(t0);

void main(uint vertexId : SV_VertexID, out PS_IN Out)
{
    static float4 vertexes[4] =
    {
        { -0.5f, -0.5f, 0.0f, 1.0f },
        {  0.5f, -0.5f, 0.0f, 1.0f },
        { -0.5f,  0.5f, 0.0f, 1.0f },
        {  0.5f,  0.5f, 0.0f, 1.0f }
    };
    
    static float4 normals[4] =
    {
        { 0.0f, 0.0f, -1.0f, 0.0f },
        { 0.0f, 0.0f, -1.0f, 0.0f },
        { 0.0f, 0.0f, -1.0f, 0.0f },
        { 0.0f, 0.0f, -1.0f, 0.0f }
    };
    
    static float4 tangents[4] =
    {
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f }
    };
    
    static float4 bitangents[4] =
    {
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f, 0.0f }
    };
    
    static float2 texCoord[4] =
    {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 0.0f, 1.0f },
        { 1.0f, 1.0f }
    };
    
    float4x4 worldMatrix = DynamicMartixArray[WorldMatrixId];
    
    Out.worldPosition = mul(vertexes[vertexId], worldMatrix);
    Out.position = mul(Out.worldPosition, ViewProjection);
    
    
    float4x4 normalWorld = worldMatrix;
    normalWorld._41_42_43 = 0.0f;
    normalWorld._44 = 1.0f;
    
    Out.normal = normalize(mul(normals[vertexId], normalWorld).xyz);
    Out.tangent = normalize(mul(tangents[vertexId], normalWorld).xyz);
    Out.bitangent = normalize(mul(bitangents[vertexId], normalWorld).xyz);
    Out.texCoord = texCoord[vertexId];
    Out.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    Out.materialId = MaterialId;
    

}