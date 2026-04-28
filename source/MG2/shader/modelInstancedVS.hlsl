// =======================================================
// Instanced Model Vertex Shader
// ・DrawIndirect + インスタンシング対応
// ・ノード階層行列の参照
// ・スキニング対応
// =======================================================

#include "common.hlsl"

// =======================================================
// バッファ定義
// =======================================================

// インスタンス情報
StructuredBuffer<MESH_INSTANCE> MeshInstanceArray : register(t0);
StructuredBuffer<MODEL_INSTANCE> ModelInstanceArray : register(t1);

// 行列データ
StructuredBuffer<BOOKMARK> MatrixDivisionBookmarks : register(t2);
StructuredBuffer<float4x4> MatrixDivisionData : register(t3);

// DrawIndirect用インデックス
StructuredBuffer<uint> MeshInstanceIndexes : register(t4);
StructuredBuffer<float4x4> DynamicMatrixArray : register(t5);
StructuredBuffer<DRAW_INDEXED_INDIRECT_ARGS> DrawArgs : register(t6);

// スキニング用
StructuredBuffer<BONE> BoneDivisionArray : register(t7);
StructuredBuffer<VERTEX_BONE_WEIGHT> VertexBoneWeightDivisionArray : register(t8);

void main(in VS_IN In, out PS_IN Out)
{
    // =======================================================
    // 頂点データ初期化
    // =======================================================
    
    float4 vertexPosition = float4(In.position, 1.0f);
    float4 vertexNormal = float4(In.normal, 0.0);
    float4 vertexTangent = float4(In.tangent, 0.0);
    float4 vertexBitangent = float4(In.bitangent, 0.0);
    
    // =======================================================
    // インスタンス情報取得
    // =======================================================
    
    // DrawIndirectのinstanceIdから実際のインスタンスIndexを取得
    uint meshInstanceIndex = MeshInstanceIndexes[DrawArgs[MeshId].startInstanceLocation + In.instanceId];
    
    MESH_INSTANCE meshInstance = MeshInstanceArray[meshInstanceIndex];
    
    // モデル単位の情報取得
    MODEL_INSTANCE modelInstance = ModelInstanceArray[meshInstance.modelInstanceId];
    
    // ノード行列の開始位置
    uint nodeMatrixOffset = MatrixDivisionBookmarks[modelInstance.matrixDivisionId].offset;
    
    // ワールド行列
    float4x4 worldMatrix = DynamicMatrixArray[modelInstance.worldMatrixId];
    
    // ノードローカル行列
    float4x4 localMatrix = MatrixDivisionData[nodeMatrixOffset + meshInstance.nodeIndex];
    
    // =======================================================
    // アニメーション行列適用
    // =======================================================
    
    if (modelInstance.animatedMatrixDivisionId != 0xffffffff)
    {
        uint animatedMatrixOffset = MatrixDivisionBookmarks[modelInstance.animatedMatrixDivisionId].offset;
        
        // アニメーション後のノード行列へ置き換え
        localMatrix = MatrixDivisionData[animatedMatrixOffset + meshInstance.nodeIndex];
        
        // ===================================================
        // スキニング
        // ===================================================
        if (Skinning)
        {
            uint boneOffset = BoneDivisionOffset;
            uint weightOffset = VertexBoneWeightDivisionOffset;
            
            // 頂点ごとのボーンウェイト取得（最大4）
            VERTEX_BONE_WEIGHT boneWeight = VertexBoneWeightDivisionArray[weightOffset + In.vertexId];
            
            uint4 boneIndeices = boneWeight.boneIndexes;
            float4 BoneWeights = boneWeight.boneWeights;
            
            // ボーン取得
            BONE bone0 = BoneDivisionArray[boneOffset + boneIndeices[0]];
            BONE bone1 = BoneDivisionArray[boneOffset + boneIndeices[1]];
            BONE bone2 = BoneDivisionArray[boneOffset + boneIndeices[2]];
            BONE bone3 = BoneDivisionArray[boneOffset + boneIndeices[3]];
        
            // ボーン変換行列を加重合成
            float4x4 skinMatrix;
            skinMatrix = mul(mul(bone0.offsetMatrix, MatrixDivisionData[animatedMatrixOffset + bone0.nodeIndex]), BoneWeights.x);
            skinMatrix += mul(mul(bone1.offsetMatrix, MatrixDivisionData[animatedMatrixOffset + bone1.nodeIndex]), BoneWeights.y);
            skinMatrix += mul(mul(bone2.offsetMatrix, MatrixDivisionData[animatedMatrixOffset + bone2.nodeIndex]), BoneWeights.z);
            skinMatrix += mul(mul(bone3.offsetMatrix, MatrixDivisionData[animatedMatrixOffset + bone3.nodeIndex]), BoneWeights.w);
            
            // 頂点位置に適用
            vertexPosition = mul(vertexPosition, skinMatrix);
        
            // 法線用に平行移動を除去
            skinMatrix._41_42_43 = 0.0f;
            skinMatrix._44 = 1.0f;
        
            // 法線・接線・従法線にも適用
            vertexNormal = float4(normalize(mul(vertexNormal, skinMatrix).xyz), 0.0f);
            vertexTangent = float4(normalize(mul(vertexTangent, skinMatrix).xyz), 0.0f);
            vertexBitangent = float4(normalize(mul(vertexBitangent, skinMatrix).xyz), 0.0f);
        }
        
    }
    
    // =======================================================
    // ワールド変換
    // =======================================================
    
    float4x4 nodeWorldMatrix = mul(localMatrix, worldMatrix);
    
    Out.worldPosition = mul(vertexPosition, nodeWorldMatrix);
    
    Out.position = mul(Out.worldPosition, ViewProjection);
    
    // =======================================================
    // 法線変換
    // =======================================================
    
    float4x4 normalWorld = nodeWorldMatrix;
    normalWorld._41_42_43 = 0.0f;
    normalWorld._44 = 1.0f;
    
    Out.normal = normalize(mul(vertexNormal, normalWorld).xyz);
    Out.tangent = normalize(mul(vertexTangent, normalWorld).xyz);
    Out.bitangent = normalize(mul(vertexBitangent, normalWorld).xyz);
    
    // =======================================================
    // その他属性
    // =======================================================
    
    Out.texCoord = In.texCoord;
    Out.color = In.color;
    Out.materialId = meshInstance.materialId;

}

