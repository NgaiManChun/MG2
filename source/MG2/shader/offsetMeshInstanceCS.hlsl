#include "common.hlsl"

RWStructuredBuffer<DRAW_INDEXED_INDIRECT_ARGS> DrawArgs : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint offset = 0;
    for (uint i = 0; i < CSMaxX; i++)
    {
        DrawArgs[i].startInstanceLocation = offset;
        offset += DrawArgs[i].instanceMaxCount;
    }

}