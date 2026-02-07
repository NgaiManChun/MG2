
#include "common.hlsl"

struct DRAW_INDEXED_INDIRECT_ARGS
{
    unsigned int indexCountPerInstance;
    unsigned int instanceCount;
    unsigned int startIndexLocation;
    int baseVertexLocation;
    unsigned int startInstanceLocation;
    unsigned int instanceMaxCount;
};

StructuredBuffer<MESH_INSTANCE> MeshInstanceArray : register(t0);
StructuredBuffer<MODEL_INSTANCE> ModelInstanceArray : register(t1);
StructuredBuffer<DISVISION_META> MatrixDivisionMeta : register(t2);
StructuredBuffer<float4x4> MatrixDivisionData : register(t3);
StructuredBuffer<uint> MeshInstanceIndexes : register(t4);
StructuredBuffer<float4x4> DynamicMatrixArray : register(t5);
StructuredBuffer<DRAW_INDEXED_INDIRECT_ARGS> DrawArgs : register(t6);

StructuredBuffer<ANIMATION_SET_RESULT> AnimationSetResultArray : register(t7);
StructuredBuffer<TRANSFORM> TransformDivisionData : register(t8);
StructuredBuffer<uint> DynamicIndexDivisionData : register(t9);

StructuredBuffer<BONE> BoneDivisionArray : register(t10);
StructuredBuffer<VERTEX_BONE_WEIGHT> VertexBoneWeightDivisionArray : register(t11);

float4x4 NodeMatrix(uint nodeIndex, uint nodeParentOffset, uint transformOffsetFrom, uint transformOffsetTo, float blend)
{
    uint _nodeIndex = nodeIndex;
    float4x4 nodeMatrix = IdentityMatrix;
    do
    {
        TRANSFORM transformFrom = TransformDivisionData[transformOffsetFrom + _nodeIndex];
        TRANSFORM transformTo = TransformDivisionData[transformOffsetTo + _nodeIndex];
        float3 position = lerp(transformFrom.position, transformTo.position, blend);
        float3 scale = lerp(transformFrom.scale, transformTo.scale, blend);
        float4 rotation = lerp(transformFrom.rotation, transformTo.rotation, blend);
        nodeMatrix = mul(nodeMatrix, MakeMatrix(position, scale, rotation));
        _nodeIndex = DynamicIndexDivisionData[nodeParentOffset + _nodeIndex];

    } while (_nodeIndex != 0xffffffff);
    return nodeMatrix;
}

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
    
    if (modelInstance.animationSetId != 0xffffffff)
    {
        ANIMATION_SET_RESULT animationSetResult = AnimationSetResultArray[modelInstance.animationSetId];
        uint nodeParentOffset = animationSetResult.nodeParentOffset;
        uint transformOffsetFrom = animationSetResult.transformOffsetFrom;
        uint transformOffsetTo = animationSetResult.transformOffsetTo;
        float blend = animationSetResult.blend;
        
        localMatrix = NodeMatrix(meshInstance.nodeIndex, nodeParentOffset, transformOffsetFrom, transformOffsetTo, blend);
        
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
            skinMatrix =  mul(mul(bone0.offsetMatrix, NodeMatrix(bone0.nodeIndex, nodeParentOffset, transformOffsetFrom, transformOffsetTo, blend)), BoneWeights.x);
            skinMatrix += mul(mul(bone1.offsetMatrix, NodeMatrix(bone1.nodeIndex, nodeParentOffset, transformOffsetFrom, transformOffsetTo, blend)), BoneWeights.y);
            skinMatrix += mul(mul(bone2.offsetMatrix, NodeMatrix(bone2.nodeIndex, nodeParentOffset, transformOffsetFrom, transformOffsetTo, blend)), BoneWeights.z);
            skinMatrix += mul(mul(bone3.offsetMatrix, NodeMatrix(bone3.nodeIndex, nodeParentOffset, transformOffsetFrom, transformOffsetTo, blend)), BoneWeights.w);
            
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

