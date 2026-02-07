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

struct DRAW_INDIRECT_ARGS
{
    uint vertexCountPerInstance;
    uint instanceCount;
    uint startVertexLocation;
    uint startInstanceLocation;
    uint instanceMaxCount;
};

RWStructuredBuffer<DRAW_INDEXED_INDIRECT_ARGS> DrawArgsIndexed : register(u0);
//RWStructuredBuffer<DRAW_INDIRECT_ARGS> DrawArgs : register(u1);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    
    if (DTid.x >= CSMaxX)
        return;
    
    DrawArgsIndexed[DTid.x].instanceCount = 0;
    //DrawArgs[DTid.x].instanceCount = 0;

}