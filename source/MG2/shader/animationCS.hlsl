// =======================================================
// Animation Compute Shader
// ・モデルインスタンスごとのアニメーション行列をGPU上で生成する
// ・現在アニメーション / 遷移元アニメーションをブレンドする
// ・結果は MatrixDivisionDataに書き込む
// =======================================================

#include "common.hlsl"

// アニメーション結果のノード行列を書き込むバッファ
RWStructuredBuffer<float4x4> MatrixDivisionData : register(u0);

// アニメーション計算対象のモデルインスタンスID
StructuredBuffer<uint> ModelInstanceIds : register(t0);

// 各種分割データの開始位置
StructuredBuffer<BOOKMARK> MatrixDivisionBookmarks : register(t1);
StructuredBuffer<BOOKMARK> TransformDivisionBookmarks : register(t2);
StructuredBuffer<BOOKMARK> DynamicIndexDivisionBookmarks : register(t3);

// モデル・アニメーション関連データ
StructuredBuffer<MODEL_INSTANCE> ModelInstanceArray : register(t4);
StructuredBuffer<ANIMATION_SET> AnimationSetArray : register(t5);
StructuredBuffer<MODEL_ANIMATION> ModelAnimationArray : register(t6);

// フレームごとのTransformデータ
StructuredBuffer<TRANSFORM> TransformDivisionData : register(t7);

// ノード親Indexデータ
StructuredBuffer<uint> DynamicIndexDivisionData : register(t8);

// 指定アニメーションの現在時刻に対応するTransform配列の先頭Offsetを取得
uint GetTransformOffset(uint modelAnimationId, uint deltaTime)
{
    MODEL_ANIMATION modelAnimation = ModelAnimationArray[modelAnimationId];
    
    // このアニメーションのTransformデータ開始位置
    uint transformOffset = TransformDivisionBookmarks[modelAnimation.transformDivisionId].offset;
    
    // ループなら時間をduration内に丸める
    // 非ループなら最終フレームで止める
    deltaTime = (modelAnimation.loop) ? deltaTime % modelAnimation.duration : min(deltaTime, modelAnimation.duration);
    
    // 経過時間からフレーム番号へ変換
    uint frame = round(float(deltaTime) / modelAnimation.duration * (modelAnimation.frameCount - 1));
    
    // フレームごとに nodeCount 個のTransformが並んでいる
    transformOffset += modelAnimation.nodeCount * frame;
    
    return transformOffset;
}

// 指定ノードの階層行列を作成する
float4x4 NodeMatrix(uint nodeIndex, uint nodeParentOffset, uint transformOffsetFrom, uint transformOffsetTo, float blend)
{
    uint _nodeIndex = nodeIndex;
    
    // 親をたどりながら行列を積む
    float4x4 nodeMatrix = IdentityMatrix;
    
    do
    {
        // 遷移元・遷移先のTransformを取得
        TRANSFORM transformFrom = TransformDivisionData[transformOffsetFrom + _nodeIndex];
        TRANSFORM transformTo = TransformDivisionData[transformOffsetTo + _nodeIndex];
        
        // 位置・スケール・回転をブレンド
        float3 position = lerp(transformFrom.position, transformTo.position, blend);
        float3 scale = lerp(transformFrom.scale, transformTo.scale, blend);
        float4 rotation = lerp(transformFrom.rotation, transformTo.rotation, blend); // ※コスト削減、回転も線形補間
        
        // 現在ノードのTransform行列を積む
        nodeMatrix = mul(nodeMatrix, MakeMatrix(position, scale, rotation));
        
        // 親ノードへ移動
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
    
    // 処理対象モデルインスタンス
    uint modelInstanceId = ModelInstanceIds[DTid.y];
    MODEL_INSTANCE modelInstance = ModelInstanceArray[modelInstanceId];
    
    // アニメーション未設定なら何もしない
    uint animationSetId = modelInstance.animationSetId;
    if (animationSetId == 0xffffffff)
        return;
    
    ANIMATION_SET animationSet = AnimationSetArray[animationSetId];
    
    // 現状は先頭アニメーションのみ使用
    // TODO: 複数アニメーションのマルチブレンド対応
    uint i = 0;
    
    // =======================================================
    // 現在時刻に対応するTransform取得
    // =======================================================
    
    // 遷移先アニメーションのTransform
    uint transformOffsetTo = GetTransformOffset(animationSet.modelAnimationIdsTo[i], CurrentTime - animationSet.animationStartTimeTo[i]);
    
    // 遷移元アニメーションのTransform
    // 遷移元がなければ、遷移先と同じTransformを使う
    uint transformOffsetFrom = (animationSet.countFrom) ?
        GetTransformOffset(animationSet.modelAnimationIdsFrom[i], CurrentTime - animationSet.animationStartTimeFrom[i]) : transformOffsetTo;
    
    // ブレンド率を計算
    float blend = float(CurrentTime - animationSet.animationBlendStartTime) / animationSet.animationBlendDuration;
    blend = saturate(blend);
    
    // ノード親Index配列の開始位置
    uint nodeParentOffset = DynamicIndexDivisionBookmarks[modelInstance.nodeParentIndexDivisionId].offset;
    
    // 書き込み先のアニメーション行列Offset
    uint matrixOffset = MatrixDivisionBookmarks[modelInstance.animatedMatrixDivisionId].offset + nodeIndex;
    
    // ノード階層を考慮した行列を生成し、結果を書き込む
    MatrixDivisionData[matrixOffset] = NodeMatrix(nodeIndex, nodeParentOffset, transformOffsetFrom, transformOffsetTo, blend);
    
}