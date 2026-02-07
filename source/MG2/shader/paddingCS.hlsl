#include "common.hlsl"

RWByteAddressBuffer Result : register(u0);

StructuredBuffer<DISVISION_META> DivisionMeta : register(t0);
ByteAddressBuffer Original : register(t1);

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    
    uint divisionIndex = DTid.y;
    uint addrOffset = DTid.x;
    
    if (divisionIndex >= CSMaxY)
        return;
    if (addrOffset >= CSMaxX)
        return;
    
    uint stride = CSMaxX; // N / 4 byte

    DISVISION_META meta = DivisionMeta[divisionIndex];
    uint begin = meta.offset;
    uint count = meta.count;
    uint padding = meta.padding;
    
    for (uint i = 0; i < count; ++i)
    {
        uint srcIndex = begin + i;
        uint dstIndex = srcIndex - padding;

        uint srcByteAddr = (srcIndex * stride + addrOffset) * 4;
        uint dstByteAddr = (dstIndex * stride + addrOffset) * 4;

        uint v = Original.Load(srcByteAddr);
        Result.Store(dstByteAddr, v);
    }

}