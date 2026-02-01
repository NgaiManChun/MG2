#include "common.hlsl"

struct ANIMATION_FOLLOWER
{
    uint dynamicMatrixId;
    uint modelInstanceId;
    uint nodeIndex;
};

RWStructuredBuffer<float4x4> DynamicMatrixArray : register(u0);

StructuredBuffer<ANIMATION_FOLLOWER> AnimationFolloerArray : register(t0);
StructuredBuffer<MODEL_INSTANCE> ModelInstanceArray : register(t1);
StructuredBuffer<ANIMATION_SET_RESULT> AnimationSetResultArray : register(t2);
StructuredBuffer<TRANSFORM> TransformDivisionData : register(t3);
StructuredBuffer<uint> DynamicIndexDivisionData : register(t4);

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

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    
    uint animationFolloerId = DTid.x;
    if (animationFolloerId >= CSMaxX)
        return;
    
    ANIMATION_FOLLOWER animationFolloer = AnimationFolloerArray[animationFolloerId];
    if (animationFolloer.dynamicMatrixId == 0xffffffff || animationFolloer.modelInstanceId == 0xffffffff)
        return;
    
    MODEL_INSTANCE modelInstance = ModelInstanceArray[animationFolloer.modelInstanceId];
    if (modelInstance.animationSetId == 0xffffffff)
        return;
    
    
    ANIMATION_SET_RESULT animationSetResult = AnimationSetResultArray[modelInstance.animationSetId];
    uint nodeParentOffset = animationSetResult.nodeParentOffset;
    uint transformOffsetFrom = animationSetResult.transformOffsetFrom;
    uint transformOffsetTo = animationSetResult.transformOffsetTo;
    float blend = animationSetResult.blend;
    
    float4x4 worldMatrix = DynamicMatrixArray[modelInstance.worldMatrixId];
    float4x4 localMatrix = NodeMatrix(animationFolloer.nodeIndex, nodeParentOffset, transformOffsetFrom, transformOffsetTo, blend);
    
    DynamicMatrixArray[animationFolloer.dynamicMatrixId] = mul(DynamicMatrixArray[animationFolloer.dynamicMatrixId]
    , mul(localMatrix, worldMatrix));
    
    
}