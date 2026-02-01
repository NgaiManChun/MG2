
#include "common.hlsl"

void main(in VS_IN In, out PS_IN Out)
{
    float4 vertexPosition = float4(In.position, 1.0f);
    float4 vertexNormal = float4(In.normal, 0.0);
    float4 vertexTangent = float4(In.tangent, 0.0);
    float4 vertexBitangent = float4(In.bitangent, 0.0);
    
    float4x4 worldMatrix = DynamicMatrix[ModelInstance[MeshInstance[In.instanceId].modelInstanceIndex].worldId];
    float4x4 localMatrix = NodeMatrix[MeshInstance[In.instanceId].modelInstanceIndex * NodeCount + NodeIndex];
    
    float4x4 nodeWorldMatrix = mul(localMatrix, worldMatrix);
    Out.worldPosition = mul(vertexPosition, nodeWorldMatrix);
    Out.position = mul(Out.worldPosition, ViewProjection);
    
    float4x4 normalWorld = nodeWorldMatrix;
    normalWorld._14_24_34 = 0.0f;
    normalWorld._44 = 1.0f;
    
    Out.normal = normalize(mul(vertexNormal, normalWorld).xyz);
    Out.tangent = normalize(mul(vertexTangent, normalWorld).xyz);
    Out.bitangent = normalize(mul(vertexBitangent, normalWorld).xyz);
    Out.texCoord = In.texCoord;
    Out.color = In.color;
    Out.materialId = MeshInstance[In.instanceId].materialId;
    
   

}

