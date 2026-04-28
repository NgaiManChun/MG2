// =======================================================
// Deferred Lighting Pixel Shader
// ・GBufferから情報を取得してライティングを計算
// ・簡易シャドウ付きのディレクショナルライト
// =======================================================

#include "common.hlsl"

// GBuffer
Texture2D ColorTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D WorldPositionTexture : register(t2);

// シャドウ関連
Texture2D DirectionalShadowMapTexture : register(t3);
Texture2D DepthTexture : register(t4);

void main(in PS_IN In, out float4 outColor : SV_Target)
{
    // =======================================================
    // GBuffer取得
    // =======================================================
    
    // 元のカラー
    outColor = ColorTexture.Sample(SamplerState0, In.texCoord);

    // 環境光
    float3 light = Ambient.rgb;
    
    // 法線取得＆正規化
    float3 normal = NormalTexture.Sample(SamplerState0, In.texCoord).xyz;
    normal = normalize(normal);
    
    // =======================================================
    // ディレクショナルライト
    // =======================================================
    
    // ライト方向との内積
    float d = dot(normal, -normalize(DirectLightDirection.xyz));
    
    // 半分を境にハードなライティング（トゥーン風）
    float3 directLight = (d > 0.5f) ? DirectLightColor.rgb : DirectLightColor.rgb * pow(saturate(d / 0.5f), 0.5);
    
    // =======================================================
    // シャドウ
    // =======================================================
    
    // サンプリング用の基準ベクトル
    float3 upper = float3(0.0f, 0.0f, 1.0f);
    
    // ライト方向に対する直交ベクトル
    float3 right = cross(DirectLightDirection.xyz, upper);
    
    // サンプル範囲を少しだけ広げる
    right *= 0.02f;
    upper *= 0.02f;
    
    // 9サンプル（中心＋周囲）
    float3 offsets[] = 
    {
        float3(0.0f, 0.0f, 0.0f),
         upper - right,
         upper + right,
        -upper - right,
        -upper + right,
        ( upper - right) * 2,
        ( upper + right) * 2,
        (-upper - right) * 2,
        (-upper + right) * 2
    };
    
    float f = 1.0f;
    
    [unroll]
    for (uint i = 0; i < 9; i++)
    {
        // ワールド座標を少しずらしてサンプリング
        float3 worldPosition = WorldPositionTexture.Sample(SamplerState0, In.texCoord).xyz + offsets[i];
        
        // シャドウマップ空間へ変換
        float4 shadowMapPosition = mul(float4(worldPosition, 1.0f), DirectionalShadowViewProjection);
        
        // NDCへ変換
        float3 ndc = shadowMapPosition.xyz / shadowMapPosition.w;
        
        // UVへ変換（Y反転）
        float2 shadowUV = float2(ndc.x * 0.5f + 0.5f, -ndc.y * 0.5f + 0.5f);
        
        // シャドウマップ範囲内のみ判定
        if (shadowUV.x < 0.98f && shadowUV.x > 0.02f &&
            shadowUV.y < 0.98f && shadowUV.y > 0.02f)
        {
            // シャドウマップの深度取得
            float z = DirectionalShadowMapTexture.Sample(SamplerState0, shadowUV).r;
            
            // 深度比較（バイアスあり）
            if (shadowMapPosition.z - 0.002f > z)
                directLight *= 0.9f; // // 影なら光を減衰
        }
    }
    
    // =======================================================
    // 最終ライティング適用
    // =======================================================
    
    light += directLight * f;
    outColor.rgb *= light;

}
