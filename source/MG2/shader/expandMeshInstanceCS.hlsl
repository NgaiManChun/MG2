// =======================================================
// Expand Mesh Instance Compute Shader
// ・モデルインスタンスから、描画用のメッシュインスタンスを生成する
// ・マテリアルID、ノード行列、AABBを展開する
// ・DrawIndirect用のインスタンス数もGPU上で加算する
// =======================================================

#include "common.hlsl"

// DrawIndexedInstancedIndirect 用の描画引数
RWStructuredBuffer<DRAW_INDEXED_INDIRECT_ARGS> DrawArgs : register(u0);

// 生成したメッシュインスタンスを書き込むAppendBuffer
AppendStructuredBuffer<MESH_INSTANCE> ResultMeshInstance : register(u1);

// 処理対象モデルインスタンスID一覧
StructuredBuffer<uint> ModelInstanceIds : register(t0);

// モデルインスタンス情報
StructuredBuffer<MODEL_INSTANCE> ModelInstanceArray : register(t1);

// マテリアルID参照
StructuredBuffer<BOOKMARK> MaterialIdDivisionBookmarks : register(t2);
StructuredBuffer<uint> MaterialIdDivisionData : register(t3);

// ノード行列参照
StructuredBuffer<BOOKMARK> MatrixDivisionBookmarks : register(t4);
StructuredBuffer<float4x4> MatrixDivisionData : register(t5);

// モデルインスタンスごとのワールド行列
StructuredBuffer<float4x4> DynamicMatrixArray : register(t6);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // インスタンス数を超えたスレッドは何もしない
    if (DTid.x >= MaxInstance)
        return;

    // 処理対象のモデルインスタンスIDを取得
    uint modelInstanceId = ModelInstanceIds[DTid.x];

    MODEL_INSTANCE modelInstance = ModelInstanceArray[modelInstanceId];

    // 無効なモデルインスタンスは描画対象にしない
    if (!modelInstance.enabled)
        return;

    // =======================================================
    // マテリアルID取得
    // =======================================================

    // このモデルインスタンスのマテリアルID配列の開始位置
    BOOKMARK materialIdDivisionMeta =
        MaterialIdDivisionBookmarks[modelInstance.materialIdDivsionId];

    // 現在のメッシュに対応するマテリアルIDを取得
    uint materialId =
        MaterialIdDivisionData[materialIdDivisionMeta.offset + MeshMaterialOffset];

    // =======================================================
    // ノード行列・ワールド行列取得
    // =======================================================

    // ノード行列配列の開始位置
    BOOKMARK nodeMatrixDivisionBookmarks =
        MatrixDivisionBookmarks[modelInstance.matrixDivisionId];

    // 現在メッシュが所属するノードのローカル行列
    float4x4 localMatrix =
        MatrixDivisionData[nodeMatrixDivisionBookmarks.offset + NodeIndex];

    // モデルインスタンスのワールド行列
    float4x4 worldMatrix =
        DynamicMatrixArray[modelInstance.worldMatrixId];

    // ノード行列とワールド行列を合成
    float4x4 nodeWorldMatrix = mul(localMatrix, worldMatrix);

    // =======================================================
    // ワールド空間AABB計算
    // =======================================================
    // ローカルAABBをそのまま8頂点変換するのではなく、
    // center + extent と行列の絶対値からワールドAABBを求める
    // =======================================================

    float3 center = (LocalMin + LocalMax) * 0.5f;
    float3 extent = (LocalMax - LocalMin) * 0.5f;

    // AABB中心をワールド変換
    float3 worldCenter =
        mul(float4(center, 1.0f), nodeWorldMatrix).xyz;

    // 回転・スケール後のAABB半径を計算
    float3 worldExtent;
    float3x3 m = (float3x3) nodeWorldMatrix;

    worldExtent.x =
        abs(m[0][0]) * extent.x +
        abs(m[1][0]) * extent.y +
        abs(m[2][0]) * extent.z;

    worldExtent.y =
        abs(m[0][1]) * extent.x +
        abs(m[1][1]) * extent.y +
        abs(m[2][1]) * extent.z;

    worldExtent.z =
        abs(m[0][2]) * extent.x +
        abs(m[1][2]) * extent.y +
        abs(m[2][2]) * extent.z;

    float3 worldMin = worldCenter - worldExtent;
    float3 worldMax = worldCenter + worldExtent;

    // =======================================================
    // メッシュインスタンス生成
    // =======================================================

    MESH_INSTANCE meshInstance;
    meshInstance.meshId = MeshId;
    meshInstance.modelInstanceId = modelInstanceId;
    meshInstance.nodeIndex = NodeIndex;
    meshInstance.materialId = materialId;
    meshInstance.min = worldMin.xyz;
    meshInstance.max = worldMax.xyz;
    meshInstance.uz = 0;

    // =======================================================
    // DrawIndirect用インスタンス数を加算
    // =======================================================

    uint meshInstanceIndex;

    // メッシュごとの最大インスタンス数をAtomic加算
    // 後段の描画・カリングで参照するためのカウント
    InterlockedAdd(
        DrawArgs[MeshId].instanceMaxCount,
        1,
        meshInstanceIndex
    );

    // 生成したメッシュインスタンスを結果バッファへ追加
    ResultMeshInstance.Append(meshInstance);
}