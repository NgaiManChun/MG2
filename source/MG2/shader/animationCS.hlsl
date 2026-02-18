#include "common.hlsl"

RWStructuredBuffer<ANIMATION_SET_RESULT> AnimationSetResultArray : register(u0);
RWStructuredBuffer<float4x4> MatrixDivisionData : register(u1);

StructuredBuffer<MODEL_INSTANCE> ModelInstanceArray : register(t0);
StructuredBuffer<ANIMATION_SET> AnimationSetArray : register(t1);
StructuredBuffer<MODEL_ANIMATION> ModelAnimationArray : register(t2);
StructuredBuffer<DISVISION_META> TransformDivisionMeta : register(t3);
StructuredBuffer<DISVISION_META> DynamicIndexDivisionMeta : register(t4);
StructuredBuffer<TRANSFORM> TransformDivisionData : register(t5);
StructuredBuffer<uint> DynamicIndexDivisionData : register(t6);
StructuredBuffer<DISVISION_META> MatrixDivisionMeta : register(t7);

uint GetTransformOffset(uint modelAnimationId, uint deltaTime)
{
    MODEL_ANIMATION modelAnimation = ModelAnimationArray[modelAnimationId];
    uint transformOffset = TransformDivisionMeta[modelAnimation.transformDivisionId].offset;
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
    
    uint instanceIndex = DTid.x;
    if (instanceIndex >= CSMaxX)
        return;
    
    MODEL_INSTANCE modelInstance = ModelInstanceArray[instanceIndex];
    uint animationSetId = modelInstance.animationSetId;
    if (animationSetId == 0xffffffff || !modelInstance.enabled)
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
    
    ANIMATION_SET_RESULT result;
    result.nodeParentOffset = DynamicIndexDivisionMeta[modelInstance.nodeParentIndexDivisionId].offset;
    result.transformOffsetFrom = transformOffsetFrom;
    result.transformOffsetTo = transformOffsetTo;
    result.blend = blend;
    
    AnimationSetResultArray[animationSetId] = result;
}