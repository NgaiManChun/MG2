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

StructuredBuffer<PARTICLE> ParticleData : register(t0);
StructuredBuffer<uint> ParticleIndexArray : register(t1);
StructuredBuffer<float4x4> DynamicMartixArray : register(t2);

void main(uint vertexId : SV_VertexID, uint instanceId : SV_InstanceID, out PS_IN Out)
{
    float4 vertexes[4] =
    {
        { -0.5f, 0.5f, 0.0f, 1.0f },
        { 0.5f, 0.5f, 0.0f, 1.0f },
        { -0.5f, -0.5f, 0.0f, 1.0f },
        { 0.5f, -0.5f, 0.0f, 1.0f }
    };
    
    float4 normals[4] =
    {
        { 0.0f, 0.0f, -1.0f, 0.0f },
        { 0.0f, 0.0f, -1.0f, 0.0f },
        { 0.0f, 0.0f, -1.0f, 0.0f },
        { 0.0f, 0.0f, -1.0f, 0.0f }
    };
    
    float4 tangents[4] =
    {
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f }
    };
    
    float4 bitangents[4] =
    {
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f, 0.0f }
    };
    
    float2 texCoord[4] =
    {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 0.0f, 1.0f },
        { 1.0f, 1.0f }
    };
    
    uint particleIndex = ParticleIndexArray[instanceId];
    PARTICLE particle = ParticleData[particleIndex];
    
    float4x4 localSR = MakeMatrix(float3(0.0f, 0.0f, 0.0f), particle.transform.scale, particle.transform.rotation);
    float4x4 localT = MakeMatrix(particle.transform.position, float3(1.0f, 1.0f, 1.0f), float4(0.0f, 0.0f, 0.0f, 1.0f));
    float4x4 localMatrix = mul(mul(localSR, InvViewRotation), localT);
    float4x4 worldMatrix = DynamicMartixArray[WorldMatrixId];
    float4x4 nodeWorldMatrix = mul(localMatrix, worldMatrix);
    
    Out.worldPosition = mul(vertexes[vertexId], nodeWorldMatrix);
    
    Out.position = mul(Out.worldPosition, ViewProjection);
    
    float4x4 normalWorld = nodeWorldMatrix;
    normalWorld._41_42_43 = 0.0f;
    normalWorld._44 = 1.0f;
    
    
    Out.normal = normalize(mul(normals[vertexId], normalWorld).xyz);
    Out.tangent = normalize(mul(tangents[vertexId], normalWorld).xyz);
    Out.bitangent = normalize(mul(bitangents[vertexId], normalWorld).xyz);
    Out.texCoord = texCoord[vertexId];
    Out.color = particle.color;
    Out.color *= particle.fade;
    Out.materialId = MaterialId;
    

}