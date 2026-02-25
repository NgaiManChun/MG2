#include "common.hlsl"

RWStructuredBuffer<float4x4> MatrixDivisionData : register(u0);

StructuredBuffer<uint> ModelInstanceIds : register(t0);
StructuredBuffer<BOOKMARK> MatrixDivisionBookmarks : register(t1);
StructuredBuffer<BOOKMARK> TransformDivisionBookmarks : register(t2);
StructuredBuffer<BOOKMARK> DynamicIndexDivisionBookmarks : register(t3);
StructuredBuffer<MODEL_INSTANCE> ModelInstanceArray : register(t4);
StructuredBuffer<ANIMATION_SET> AnimationSetArray : register(t5);
StructuredBuffer<MODEL_ANIMATION> ModelAnimationArray : register(t6);
StructuredBuffer<TRANSFORM> TransformDivisionData : register(t7);
StructuredBuffer<uint> DynamicIndexDivisionData : register(t8);


uint GetTransformOffset(uint modelAnimationId, uint deltaTime)
{
    MODEL_ANIMATION modelAnimation = ModelAnimationArray[modelAnimationId];
    uint transformOffset = TransformDivisionBookmarks[modelAnimation.transformDivisionId].offset;
    deltaTime = (modelAnimation.loop) ? deltaTime % modelAnimation.duration : min(deltaTime, modelAnimation.duration);
    uint frame = round(float(deltaTime) / modelAnimation.duration * (modelAnimation.frameCount - 1));
    transformOffset += modelAnimation.nodeCount * frame;
    return transformOffset;
}

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
    
    uint nodeIndex = DTid.x;
    if (nodeIndex >= CSMaxX)
        return;
    
    uint modelInstanceId = ModelInstanceIds[DTid.y];
    MODEL_INSTANCE modelInstance = ModelInstanceArray[modelInstanceId];
    uint animationSetId = modelInstance.animationSetId;
    if (animationSetId == 0xffffffff)
        return;
    
    ANIMATION_SET animationSet = AnimationSetArray[animationSetId];
    
    uint i = 0; // TODO: マルチブレンド
    
    // アニメーションをブレンドする前に、
    // 時間経過により、現在フレームとそのトランスフォームのアドレスを探す
    uint transformOffsetTo = GetTransformOffset(animationSet.modelAnimationIdsTo[i], CurrentTime - animationSet.animationStartTimeTo[i]);
    uint transformOffsetFrom = (animationSet.countFrom) ?
        GetTransformOffset(animationSet.modelAnimationIdsFrom[i], CurrentTime - animationSet.animationStartTimeFrom[i]) : transformOffsetTo;
    float blend = float(CurrentTime - animationSet.animationBlendStartTime) / animationSet.animationBlendDuration;
    blend = saturate(blend);
    uint nodeParentOffset = DynamicIndexDivisionBookmarks[modelInstance.nodeParentIndexDivisionId].offset;
    uint matrixOffset = MatrixDivisionBookmarks[modelInstance.animatedMatrixDivisionId].offset + nodeIndex;
    MatrixDivisionData[matrixOffset] = NodeMatrix(nodeIndex, nodeParentOffset, transformOffsetFrom, transformOffsetTo, blend);
    
}