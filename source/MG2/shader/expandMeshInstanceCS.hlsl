#include "common.hlsl"

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
    
    // ワールド行列を適用後のAABBを計算
    float3 center = (LocalMin + LocalMax) * 0.5f;
    float3 extent = (LocalMax - LocalMin) * 0.5f;
    float3 worldCenter = mul(float4(center, 1.0f), nodeWorldMatrix).xyz;
    float3 worldExtent;
    float3x3 m = (float3x3) nodeWorldMatrix;
    worldExtent.x =
    abs(m[0][0]) * extent.x +
    abs(m[1][0]) * extent.y +
    abs(m[2][0]) * extent.z;
    worldExtent.y =
    abs(m[0][1]) * extent.x +
    abs(m[1][1]) * extent.y +
    abs(m[2][1]) * extent.z;
    worldExtent.z =
    abs(m[0][2]) * extent.x +
    abs(m[1][2]) * extent.y +
    abs(m[2][2]) * extent.z;
    float3 worldMin = worldCenter - worldExtent;
    float3 worldMax = worldCenter + worldExtent;
    
    MESH_INSTANCE meshInstance;
    meshInstance.meshId = MeshId;
    meshInstance.modelInstanceId = modelInstanceId;
    meshInstance.nodeIndex = NodeIndex;
    meshInstance.materialId = materialId;
    meshInstance.min = worldMin.xyz;
    meshInstance.max = worldMax.xyz;
    meshInstance.uz = 0;
    
    uint meshInstanceIndex;
    InterlockedAdd(DrawArgs[MeshId].instanceMaxCount, 1, meshInstanceIndex);
    ResultMeshInstance.Append(meshInstance);

}