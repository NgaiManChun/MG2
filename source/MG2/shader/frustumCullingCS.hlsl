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
RWStructuredBuffer<MESH_INSTANCE> ResultMeshInstance : register(u1);
RWStructuredBuffer<uint> ResultMeshInstanceIndex : register(u2);

StructuredBuffer<MODEL_INSTANCE> ModelInstanceArray : register(t0);
StructuredBuffer<float4x4> DynamicMatrixArray : register(t1);

uint OrderedUint(float f)
{
    uint uvalue = asuint(f);
    uvalue = ((uvalue >> 31) == 0) ? (uvalue | (1 << 31)) : (~uvalue);
    return uvalue;
}

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    
    if (DTid.x >= CSMaxX)
        return;
    
    
    MESH_INSTANCE meshInstance = ResultMeshInstance[DTid.x];
    
    // LODƒeƒXƒg
    if (ForceLOD == 0)
    {
        MODEL_INSTANCE modelInstance = ModelInstanceArray[meshInstance.modelInstanceId];
        float4x4 worldMatrix = DynamicMatrixArray[modelInstance.worldMatrixId];
        float3 position = worldMatrix._41_42_43;
        float modelZ = dot(CameraForward, position - CameraPosition);
    
        // cFOV
        float f = Projection._22;
        // “Š‰ek¬—¦
        float scale = f / max(modelZ, 0.0001f);
        uint lod = 0x10;
        if (scale > 0.5f)
            lod = 0x1;
        else if (scale > 0.25f)
            lod = 0x2;
        else if (scale > 0.125f)
            lod = 0x4;
        else if (scale > 0.0625f)
            lod = 0x8;
    
        if (!bool(ModelInstanceArray[meshInstance.modelInstanceId].lod & lod))
            return;
    }
    else
    {
        if (!bool(ModelInstanceArray[meshInstance.modelInstanceId].lod & ForceLOD))
            return;
    }
    
    float3 center = lerp(meshInstance.min, meshInstance.max, 0.5f);
    float radius = length(center - meshInstance.max);
    float radiusSq = (radius > 1.0f) ? radius * radius * radius : 1.0f;
    float3 v = center - CameraPosition;
    float z = dot(CameraForward, v);
    
    // z”ÍˆÍƒeƒXƒg
    if (z < 0.0f && z * z > radiusSq)
        return;
    if ((z > ProjectionFar && ((z - ProjectionFar) * (z - ProjectionFar) > radiusSq)))
        return;
    
    bool fail = false;
    
    [unroll]
    for (uint i = 0; i < 4; i++)
    {
        float d = dot(Frustum[i].xyz, center - FrustumPoints[i].xyz);
        fail = (d < -2.0f && d * d > radiusSq) ? true : fail;
    }
    
    if (fail)
        return;
    
    uint count;
    InterlockedAdd(DrawArgs[meshInstance.meshId].instanceCount, 1, count);
    ResultMeshInstanceIndex[DrawArgs[meshInstance.meshId].startInstanceLocation + count] = DTid.x;
    
    

}