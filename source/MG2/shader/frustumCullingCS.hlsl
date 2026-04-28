// =======================================================
// Frustum Culling Compute Shader
// ・展開済みのメッシュインスタンスを視錐台カリングする
// ・LOD判定もここで行う
// ・描画対象だけをDrawIndirect用Indexバッファへ登録する
// =======================================================

#include "common.hlsl"

// DrawIndexedInstancedIndirect 用の描画引数
RWStructuredBuffer<DRAW_INDEXED_INDIRECT_ARGS> DrawArgs : register(u0);

// expandMeshInstanceCSで生成されたメッシュインスタンス一覧
RWStructuredBuffer<MESH_INSTANCE> ResultMeshInstance : register(u1);

// カリング後に描画するメッシュインスタンスIndex
RWStructuredBuffer<uint> ResultMeshInstanceIndex : register(u2);

// モデルインスタンス情報
StructuredBuffer<MODEL_INSTANCE> ModelInstanceArray : register(t0);

// モデルインスタンスごとのワールド行列
StructuredBuffer<float4x4> DynamicMatrixArray : register(t1);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // 処理対象数を超えたスレッドは何もしない
    if (DTid.x >= CSMaxX)
        return;

    // 対象メッシュインスタンス取得
    MESH_INSTANCE meshInstance = ResultMeshInstance[DTid.x];

    // =======================================================
    // LOD判定
    // =======================================================

    if (ForceLOD == 0)
    {
        MODEL_INSTANCE modelInstance =
            ModelInstanceArray[meshInstance.modelInstanceId];

        float4x4 worldMatrix =
            DynamicMatrixArray[modelInstance.worldMatrixId];

        // モデルのワールド位置
        float3 position = worldMatrix._41_42_43;

        // カメラ前方向に対する距離
        float modelZ = dot(CameraForward, position - CameraPosition);

        // Projection._22 は縦方向FOV由来の係数
        float f = Projection._22;

        // 画面上での見かけの大きさに近い値
        float scale = f / max(modelZ, 0.0001f);

        // 距離に応じて使用するLODビットを決定
        uint lod = 0x10;

        if (scale > 0.5f)
            lod = 0x1;
        else if (scale > 0.25f)
            lod = 0x2;
        else if (scale > 0.125f)
            lod = 0x4;
        else if (scale > 0.0625f)
            lod = 0x8;

        // このモデルインスタンスが該当LODを持っていなければ描画しない
        if (!bool(ModelInstanceArray[meshInstance.modelInstanceId].lod & lod))
            return;
    }
    else
    {
        // ForceLOD指定時は、そのLODだけを描画対象にする
        if (!bool(ModelInstanceArray[meshInstance.modelInstanceId].lod & ForceLOD))
            return;
    }

    // =======================================================
    // バウンディング球作成
    // =======================================================
    // AABBから中心と半径を求め、簡易的に視錐台判定する
    // =======================================================

    float3 center = lerp(meshInstance.min, meshInstance.max, 0.5f);
    float radius = length(center - meshInstance.max);

    // 半径判定用
    // 意図的に少し判定を広く計算
    float radiusSq = radius * radius * radius;

    // カメラから中心へのベクトル
    float3 v = center - CameraPosition;

    // カメラ前方向の距離
    float z = dot(CameraForward, v);

    // =======================================================
    // Near / Far 方向の簡易カリング
    // =======================================================

    // カメラ後方に完全にある場合は除外
    if (z < 0.0f && z * z > radiusSq)
        return;

    // Far平面より奥に完全にある場合は除外
    if (z > ProjectionFar && ((z - ProjectionFar) * (z - ProjectionFar) > radiusSq))
        return;

    // =======================================================
    // 視錐台左右上下平面テスト
    // =======================================================

    bool fail = false;

    [unroll]
    for (uint i = 0; i < 4; i++)
    {
        // 平面との距離
        float d = dot(Frustum[i].xyz, center - FrustumPoints[i].xyz);

        // 球が平面の外側に完全に出ていれば除外
        fail = (d < -2.0f && d * d > radiusSq) ? true : fail;
    }

    if (fail)
        return;

    // =======================================================
    // 描画対象として登録
    // =======================================================

    uint count;

    // メッシュごとの描画インスタンス数をAtomic加算
    InterlockedAdd(DrawArgs[meshInstance.meshId].instanceCount, 1, count);

    // DrawArgsのstartInstanceLocationを基準に、
    // カリングを通過したメッシュインスタンスIndexを書き込む
    ResultMeshInstanceIndex[
        DrawArgs[meshInstance.meshId].startInstanceLocation + count
    ] = DTid.x;
}