// =======================================================
// Animation Follow Compute Shader
// ・指定ノードのアニメーション行列を参照して
//   別のDynamicMatrixへ追従させる（アタッチ用）
// ・例：手に持つ武器、エフェクト、子オブジェクトなど
// =======================================================

#include "common.hlsl"

// フォロワー設定
struct ANIMATION_FOLLOWER
{
    uint dynamicMatrixId; // 書き込み先（追従対象のワールド行列）
    uint modelInstanceId; // 参照元モデルインスタンス
    uint nodeIndex; // 追従するノードIndex
};

// ワールド行列（インスタンス単位）
RWStructuredBuffer<float4x4> DynamicMatrixArray : register(u0);

// フォロワー一覧
StructuredBuffer<ANIMATION_FOLLOWER> AnimationFolloerArray : register(t0);

// モデル情報
StructuredBuffer<MODEL_INSTANCE> ModelInstanceArray : register(t1);

// アニメーション行列参照
StructuredBuffer<BOOKMARK> MatrixDivisionBookmarks : register(t2);
StructuredBuffer<float4x4> MatrixDivisionData : register(t3);


[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // X方向：フォロワーID
    uint animationFollowerId = DTid.x;

    // 範囲外スレッドは終了
    if (animationFollowerId >= CSMaxX)
        return;

    ANIMATION_FOLLOWER animationFollower =
        AnimationFolloerArray[animationFollowerId];

    // 無効IDなら処理しない
    if (animationFollower.dynamicMatrixId == 0xffffffff ||
        animationFollower.modelInstanceId == 0xffffffff)
        return;

    MODEL_INSTANCE modelInstance =
        ModelInstanceArray[animationFollower.modelInstanceId];

    // アニメーション未設定なら何もしない
    if (modelInstance.animationSetId == 0xffffffff)
        return;

    // =======================================================
    // 行列取得
    // =======================================================

    // モデルのワールド行列
    float4x4 worldMatrix =
        DynamicMatrixArray[modelInstance.worldMatrixId];

    // アニメーション済みノード行列の開始位置
    BOOKMARK matrixDivisionBookmark =
        MatrixDivisionBookmarks[modelInstance.animatedMatrixDivisionId];

    // 対象ノードのローカル行列（アニメーション適用済）
    float4x4 localMatrix =
        MatrixDivisionData[matrixDivisionBookmark.offset + animationFollower.nodeIndex];

    // =======================================================
    // フォロー処理
    // =======================================================

    // 既存の行列に対して
    // （ノードローカル × ワールド）を乗算して追従させる
    DynamicMatrixArray[animationFollower.dynamicMatrixId] =
        mul(
            DynamicMatrixArray[animationFollower.dynamicMatrixId],
            mul(localMatrix, worldMatrix)
        );
}