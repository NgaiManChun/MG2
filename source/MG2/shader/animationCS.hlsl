#include "common.hlsl"

RWStructuredBuffer<ANIMATION_SET_RESULT> AnimationSetResultArray : register(u0);

StructuredBuffer<MODEL_INSTANCE> ModelInstanceArray : register(t0);
StructuredBuffer<ANIMATION_SET> AnimationSetArray : register(t1);
StructuredBuffer<MODEL_ANIMATION> ModelAnimationArray : register(t2);
StructuredBuffer<DISVISION_META> TransformDivisionMeta : register(t3);
StructuredBuffer<DISVISION_META> DynamicIndexDivisionMeta : register(t4);

uint GetTransformOffset(uint modelAnimationId, uint deltaTime)
{
    MODEL_ANIMATION modelAnimation = ModelAnimationArray[modelAnimationId];
    uint transformOffset = TransformDivisionMeta[modelAnimation.transformDivisionId].offset;
    deltaTime = (modelAnimation.loop) ? deltaTime % modelAnimation.duration : min(deltaTime, modelAnimation.duration);
    uint frame = round(float(deltaTime) / modelAnimation.duration * (modelAnimation.frameCount - 1));
    transformOffset += modelAnimation.nodeCount * frame;
    return transformOffset;

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