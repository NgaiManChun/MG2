#include "common.hlsl"

RWStructuredBuffer<DRAW_INDEXED_INDIRECT_ARGS> DrawArgsIndexed : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    
    if (DTid.x >= CSMaxX)
        return;
    
    DrawArgsIndexed[DTid.x].instanceCount = 0;

}