#include "common.hlsl"

struct PARTICLE
{
    TRANSFORM transform;
    float3 velocity;
    float3 acceleration;
    float4 color;
    float fade;
    float life;
};

struct DRAW_INDIRECT_ARGS
{
    uint vertexCountPerInstance;
    uint instanceCount;
    uint startVertexLocation;
    uint startInstanceLocation;
    uint instanceMaxCount;
};

cbuffer ParticleBuffer : register(b6)
{
    float4 INIT_POSITION_RANGE;
    float4 INIT_SCALE;
    float4 INIT_SCALE_ACCELERATION;
    float4 INIT_VELOCITY;
    float4 INIT_ACCELERATION;
    float4 INIT_COLOR;
    float INIT_LIFE_MIN;
    float INIT_LIFE_MAX;
    float FADE_IN_TIME;
    float FADE_OUT_TIME;
}

RWStructuredBuffer<DRAW_INDIRECT_ARGS> DrawArgs : register(u0);
RWStructuredBuffer<PARTICLE> ParticleData : register(u1);
RWStructuredBuffer<int> SpawnCount : register(u2);
AppendStructuredBuffer<uint> ParticleIndexArray : register(u3);


[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    
    if (DTid.x >= CSMaxX)
        return;
    
    uint argsOffset = CSMaxY;
    uint nowTime = CSMaxZ;
    float deltaTime = asfloat(CSMaxW);
    int seed = nowTime + DTid.x + 100 * DTid.x;
    
    PARTICLE particle = ParticleData[DTid.x];
    
    if (particle.life == 0.0f)
    {
        int count;
        InterlockedAdd(SpawnCount[0], -1, count);
        if (count > 0)
        {
            
            // スポーン
            particle.life = INIT_LIFE_MIN + (INIT_LIFE_MAX - INIT_LIFE_MIN) * Random(++seed);
            
            particle.transform.position =
            INIT_POSITION_RANGE.xyz *
            float3(Random(++seed), Random(++seed), Random(++seed)) *
            float3(GetRandomSign(++seed), GetRandomSign(++seed), GetRandomSign(++seed));
            
            particle.transform.scale = INIT_SCALE;
            
            particle.velocity = INIT_VELOCITY.xyz *
            float3(Random(++seed), Random(++seed), Random(++seed)) *
            float3(GetRandomSign(++seed), GetRandomSign(++seed), GetRandomSign(++seed));
            
            particle.acceleration = INIT_ACCELERATION.xyz *
            float3(Random(++seed), Random(++seed), Random(++seed)) *
            float3(GetRandomSign(++seed), GetRandomSign(++seed), GetRandomSign(++seed));
            
            particle.color = INIT_COLOR;
            particle.fade = 0.0f;
            
            ParticleData[DTid.x] = particle;

        }
    }
    
    if (particle.life > 0.0f)
    {
        // 更新
        particle.transform.position += particle.velocity * deltaTime;
        particle.velocity += particle.acceleration * deltaTime;
        
        if (particle.life > FADE_OUT_TIME)
        {
            particle.fade = saturate(particle.fade + deltaTime / FADE_IN_TIME);
        }
        else
        {
            particle.fade = saturate(particle.fade - deltaTime);
        }
        particle.transform.scale += INIT_SCALE_ACCELERATION.xyz * deltaTime;
        particle.life = max(particle.life - deltaTime, 0.0f);
        ParticleData[DTid.x] = particle;
        
        // 描画確保
        ParticleIndexArray.Append(DTid.x);
        InterlockedAdd(DrawArgs[argsOffset].instanceCount, 1);
    }
    

}