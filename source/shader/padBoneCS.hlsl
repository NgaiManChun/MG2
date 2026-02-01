#include "common.hlsl"

RWStructuredBuffer<BONE> Result : register(u0);

StructuredBuffer<DISVISION_META> DivisionMeta : register(t0);
StructuredBuffer<BONE> Original : register(t1);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    
    uint divisionIndex = DTid.x;
    if (divisionIndex >= CSMaxX)
        return;
    
    DISVISION_META meta = DivisionMeta[divisionIndex];
    uint offset = meta.offset;
    uint count = meta.count;
    uint padding = meta.padding;
    
    for (uint i = offset; i < offset + count; i++)
    {
        Result[i - padding] = Original[i];
    }

}