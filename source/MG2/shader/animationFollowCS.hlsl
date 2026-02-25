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
StructuredBuffer<DISVISION_META> MatrixDivisionMeta : register(t2);
StructuredBuffer<float4x4> MatrixDivisionData : register(t3);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    
    uint animationFollowerId = DTid.x;
    if (animationFollowerId >= CSMaxX)
        return;
    
    ANIMATION_FOLLOWER animationFollower = AnimationFolloerArray[animationFollowerId];
    if (animationFollower.dynamicMatrixId == 0xffffffff || animationFollower.modelInstanceId == 0xffffffff)
        return;
    
    MODEL_INSTANCE modelInstance = ModelInstanceArray[animationFollower.modelInstanceId];
    if (modelInstance.animationSetId == 0xffffffff)
        return;
    
    float4x4 worldMatrix = DynamicMatrixArray[modelInstance.worldMatrixId];
    DISVISION_META matrixDivisionBookmark = MatrixDivisionMeta[modelInstance.animatedMatrixDivisionId];
    float4x4 localMatrix = MatrixDivisionData[matrixDivisionBookmark.offset + animationFollower.nodeIndex];
    
    DynamicMatrixArray[animationFollower.dynamicMatrixId] = 
        mul(DynamicMatrixArray[animationFollower.dynamicMatrixId], mul(localMatrix, worldMatrix));
    
    
}