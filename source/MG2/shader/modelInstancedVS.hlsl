
#include "common.hlsl"

StructuredBuffer<MESH_INSTANCE> MeshInstanceArray : register(t0);
StructuredBuffer<MODEL_INSTANCE> ModelInstanceArray : register(t1);
StructuredBuffer<DISVISION_META> MatrixDivisionMeta : register(t2);
StructuredBuffer<float4x4> MatrixDivisionData : register(t3);
StructuredBuffer<uint> MeshInstanceIndexes : register(t4);
StructuredBuffer<float4x4> DynamicMatrixArray : register(t5);
StructuredBuffer<DRAW_INDEXED_INDIRECT_ARGS> DrawArgs : register(t6);

StructuredBuffer<BONE> BoneDivisionArray : register(t7);
StructuredBuffer<VERTEX_BONE_WEIGHT> VertexBoneWeightDivisionArray : register(t8);

void main(in VS_IN In, out PS_IN Out)
{
    float4 vertexPosition = float4(In.position, 1.0f);
    float4 vertexNormal = float4(In.normal, 0.0);
    float4 vertexTangent = float4(In.tangent, 0.0);
    float4 vertexBitangent = float4(In.bitangent, 0.0);
    
    uint meshInstanceIndex = MeshInstanceIndexes[DrawArgs[MeshId].startInstanceLocation + In.instanceId];
    MESH_INSTANCE meshInstance = MeshInstanceArray[meshInstanceIndex];
    MODEL_INSTANCE modelInstance = ModelInstanceArray[meshInstance.modelInstanceId];
    uint nodeMatrixOffset = MatrixDivisionMeta[modelInstance.matrixDivisionId].offset;
    
    float4x4 worldMatrix = DynamicMatrixArray[modelInstance.worldMatrixId];
    float4x4 localMatrix = MatrixDivisionData[nodeMatrixOffset + meshInstance.nodeIndex];
    
    if (modelInstance.animatedMatrixDivisionId != 0xffffffff)
    {
        uint animatedMatrixOffset = MatrixDivisionMeta[modelInstance.animatedMatrixDivisionId].offset;
        localMatrix = MatrixDivisionData[animatedMatrixOffset + meshInstance.nodeIndex];
        
        if (Skinning)
        {
            uint boneOffset = BoneDivisionOffset;
            uint weightOffset = VertexBoneWeightDivisionOffset;
            VERTEX_BONE_WEIGHT boneWeight = VertexBoneWeightDivisionArray[weightOffset + In.vertexId];
            uint4 boneIndeices = boneWeight.boneIndexes;
            float4 BoneWeights = boneWeight.boneWeights;
            BONE bone0 = BoneDivisionArray[boneOffset + boneIndeices[0]];
            BONE bone1 = BoneDivisionArray[boneOffset + boneIndeices[1]];
            BONE bone2 = BoneDivisionArray[boneOffset + boneIndeices[2]];
            BONE bone3 = BoneDivisionArray[boneOffset + boneIndeices[3]];
        
            float4x4 skinMatrix;
            skinMatrix = mul(mul(bone0.offsetMatrix, MatrixDivisionData[animatedMatrixOffset + bone0.nodeIndex]), BoneWeights.x);
            skinMatrix += mul(mul(bone1.offsetMatrix, MatrixDivisionData[animatedMatrixOffset + bone1.nodeIndex]), BoneWeights.y);
            skinMatrix += mul(mul(bone2.offsetMatrix, MatrixDivisionData[animatedMatrixOffset + bone2.nodeIndex]), BoneWeights.z);
            skinMatrix += mul(mul(bone3.offsetMatrix, MatrixDivisionData[animatedMatrixOffset + bone3.nodeIndex]), BoneWeights.w);
            
            vertexPosition = mul(vertexPosition, skinMatrix);
        
            skinMatrix._41_42_43 = 0.0f;
            skinMatrix._44 = 1.0f;
        
            vertexNormal = float4(normalize(mul(vertexNormal, skinMatrix).xyz), 0.0f);
            vertexTangent = float4(normalize(mul(vertexTangent, skinMatrix).xyz), 0.0f);
            vertexBitangent = float4(normalize(mul(vertexBitangent, skinMatrix).xyz), 0.0f);
        }
        
    }
    
    float4x4 nodeWorldMatrix = mul(localMatrix, worldMatrix);
    
    Out.worldPosition = mul(vertexPosition, nodeWorldMatrix);
    
    Out.position = mul(Out.worldPosition, ViewProjection);
    
    float4x4 normalWorld = nodeWorldMatrix;
    normalWorld._41_42_43 = 0.0f;
    normalWorld._44 = 1.0f;
    
    Out.normal = normalize(mul(vertexNormal, normalWorld).xyz);
    Out.tangent = normalize(mul(vertexTangent, normalWorld).xyz);
    Out.bitangent = normalize(mul(vertexBitangent, normalWorld).xyz);
    Out.texCoord = In.texCoord;
    Out.color = In.color;
    Out.materialId = meshInstance.materialId;

}

