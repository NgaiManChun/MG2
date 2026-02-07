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

RWStructuredBuffer<DRAW_INDEXED_INDIRECT_ARGS> DrawArgs : register(u0);
AppendStructuredBuffer<MESH_INSTANCE> ResultMeshInstance : register(u1);

StructuredBuffer<uint> ModelInstanceIds : register(t0);
StructuredBuffer<MODEL_INSTANCE> ModelInstanceArray : register(t1);
StructuredBuffer<DISVISION_META> MaterialIdDivisionMeta : register(t2);
StructuredBuffer<uint> MaterialIdDivisionData : register(t3);
StructuredBuffer<DISVISION_META> MatrixDivisionMeta : register(t4);
StructuredBuffer<float4x4> MatrixDivisionData : register(t5);
StructuredBuffer<float4x4> DynamicMatrixArray : register(t6);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= MaxInstance)
        return;
    
    uint modelInstanceId = ModelInstanceIds[DTid.x];
    
    MODEL_INSTANCE modelInstance = ModelInstanceArray[modelInstanceId];
    if (!modelInstance.enabled)
        return;
    
    DISVISION_META materialIdDivisionMeta = MaterialIdDivisionMeta[modelInstance.materialIdDivsionId];
    uint materialId = MaterialIdDivisionData[materialIdDivisionMeta.offset + MeshMaterialOffset];
    
    DISVISION_META nodeMatrixDivisionMeta = MatrixDivisionMeta[modelInstance.matrixDivisionId];
    float4x4 localMatrix = MatrixDivisionData[nodeMatrixDivisionMeta.offset + NodeIndex];
    
    float4x4 worldMatrix = DynamicMatrixArray[modelInstance.worldMatrixId];
    
    float4x4 nodeWorldMatrix = mul(localMatrix, worldMatrix);
    
    float4 position0 = mul(float4(LocalMin, 1.0f), nodeWorldMatrix);
    float4 position1 = mul(float4(LocalMax, 1.0f), nodeWorldMatrix);
    float3 worldMin = float3(min(position0.x, position1.x), min(position0.y, position1.y), min(position0.z, position1.z));
    float3 worldMax = float3(max(position0.x, position1.x), max(position0.y, position1.y), max(position0.z, position1.z));
    
    MESH_INSTANCE meshInstance;
    meshInstance.meshId = MeshId;
    meshInstance.modelInstanceId = modelInstanceId;
    meshInstance.nodeIndex = NodeIndex;
    meshInstance.materialId = materialId;
    meshInstance.min = worldMin.xyz;
    meshInstance.max = worldMax.xyz;
    meshInstance.uz = 0;
    
    //uint DRAW_ARGS_SIZE = 24;
    //uint MAX_COUNT_ARG_OFFSET = 20;
    uint meshInstanceIndex;
    InterlockedAdd(DrawArgs[MeshId].instanceMaxCount, 1, meshInstanceIndex);
    //DrawArgs.InterlockedAdd(MeshId * DRAW_ARGS_SIZE + MAX_COUNT_ARG_OFFSET, 1, meshInstanceIndex);
    ResultMeshInstance.Append(meshInstance);

}