
struct VERTEX
{
    float3 position;
    float3 normal;
    float3 tangent;
    float3 bitangent;
    float2 texCoord;
    float4 color;
};

struct TRANSFORM
{
    float3 position;
    float3 scale;
    float4 rotation;
};

struct VERTEX_BONE_WEIGHT
{
    uint4 boneIndexes;
    float4 boneWeights;
};

struct BONE
{
    float4x4 offsetMatrix;
    uint nodeIndex;
};

struct MATERIAL
{
    float4 base;
    float4 emissive;
    float4 specular;
    float metallic;
    float roughness;
    float shininess;
    uint baseTextureId;
    uint normalTextureId;
    uint opacityTextureId;
    uint type;
    uint opaque;
};

struct MODEL_INSTANCE
{
    uint modelId;
    uint materialIdDivsionId;
    uint worldMatrixId;
    uint enabled;
    uint animationSetId;
    uint matrixDivisionId;
    uint nodeParentIndexDivisionId;
    uint lod;
};

struct MESH_INSTANCE
{
    uint meshId;
    uint modelInstanceId;
    uint nodeIndex;
    uint materialId;
    float3 min;
    float3 max;
    uint uz;
};

struct DRAW_INSTANCE
{
    uint modelInstanceId;
    uint nodeIndex;
    uint materialId;
    
};

struct DISVISION_META
{
    uint offset;
    uint count;
    uint padding;
};

struct MODEL_ANIMATION
{
    uint modelId;
    uint animationId;
    uint transformDivisionId;
    uint nodeCount;
    uint frameCount;
    uint duration;
    uint loop;
};

struct ANIMATION_SET
{
    uint modelAnimationIdsFrom[1];
    uint animationStartTimeFrom[1];
    uint countFrom;
    uint modelAnimationIdsTo[1];
    uint animationStartTimeTo[1];
    uint countTo;
    uint animationBlendDuration;
    uint animationBlendStartTime;
    float timeMultiplier;
};

struct ANIMATION_SET_RESULT
{
    uint nodeParentOffset;
    uint transformOffsetFrom;
    uint transformOffsetTo;
    float blend;
};

struct SPOT_LIGHT
{
    matrix ViewProjection;
    float3 Position;
    float Destance;
    float3 Color;
    float Blur;
};

struct VS_IN
{
    float3 position : POSITION0;
    float3 normal : NORMAL0;
    float3 tangent : TANGENT0;
    float3 bitangent : BINORMAL0;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR0;
    
    uint instanceId : SV_InstanceID;
    uint vertexId : SV_VertexID;
};

struct VS_IN_DIRECT
{
    uint meshId: MESHID0;
    uint modelInstanceId: MODELINSTANCEID0;
    uint localId: LOCALID0;
    uint materialId: MATERIALID0;
    
    // 自動生成
    uint instanceId : SV_InstanceID;
    uint vertexId : SV_VertexID;
};

struct PS_IN
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL0;
    float3 tangent : TANGENT0;
    float3 bitangent : BINORMAL0;
    float2 texCoord : TEXCOORD0;
    float4 color : COLOR0;
    float4 worldPosition : POSITION0;
    nointerpolation uint materialId : MATERIALID0;
};

cbuffer CameraBuffer : register(b0)
{
    matrix View;
    matrix Projection;
    matrix ViewProjection;
    matrix InvViewProjection;
    matrix InvViewRotation;
    float4 Frustum[4];
    float3 CameraPosition;
    float ProjectionFar;
    uint ForceLOD;
    float3 CameraForward;
    float4 FrustumPoints[4];
}

cbuffer LightBuffer : register(b1)
{
    float4 Ambient;
    float4 DirectLightColor;
    float4 DirectLightDirection;
    matrix DirectionalShadowViewProjection;
}

cbuffer ModelBuffer : register(b2)
{
    uint ModelId;
    uint ModelMaterialCount;
    uint NodeCount;
    uint MaxInstance;
    uint NodeMatrixDivisionId;
    uint NodeParentIndexDivisionOffset;
    uint Animating;
}

cbuffer MeshBuffer : register(b3)
{
    uint MeshId;
    uint MeshMaterialOffset;
    uint NodeIndex;
    uint Skinning;
    uint BoneDivisionOffset;
    uint VertexBoneWeightDivisionOffset;
    float3 LocalMin;
    float3 LocalMax;
}

cbuffer CSConstantBuffer : register(b4)
{
    uint CSMaxX;
    uint CSMaxY;
    uint CSMaxZ;
    uint CSMaxW;
}

cbuffer SingleBuffer : register(b5)
{
    uint WorldMatrixId;
    uint LocalMatrixDivisionOffset;
    uint MaterialId;
}

// cbuffer ParticleBuffer : register(b6)

cbuffer TimeBuffer : register(b7)
{
    uint CurrentTime;
    uint DeltaTime;
}

static float4x4 IdentityMatrix = 
float4x4(
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
);

uint FloatToSortableUint(float f)
{
    uint u = asuint(f);
    uint mask = (u & 0x80000000) ? 0xFFFFFFFF : 0x80000000;
    return u ^ mask;
}

float3x3 QuaternionToMatrix(float4 q)
{
    float x = q.x, y = q.y, z = q.z, w = q.w;

    float xx = x * x;
    float yy = y * y;
    float zz = z * z;
    float xy = x * y;
    float xz = x * z;
    float yz = y * z;
    float wx = w * x;
    float wy = w * y;
    float wz = w * z;

    return float3x3(
        1 - 2 * (yy + zz), 2 * (xy + wz), 2 * (xz - wy),
        2 * (xy - wz), 1 - 2 * (xx + zz), 2 * (yz + wx),
        2 * (xz + wy), 2 * (yz - wx), 1 - 2 * (xx + yy)
    );
}

float4x4 MakeMatrix(float3 position, float3 scale, float4 rotation)
{
    float3x3 R = QuaternionToMatrix(rotation);

    // スケールを回転行列に掛ける（各軸）
    R[0] *= scale.x;
    R[1] *= scale.y;
    R[2] *= scale.z;

    return float4x4(
        float4(R[0], 0),
        float4(R[1], 0),
        float4(R[2], 0),
        float4(position, 1)
    );
}

uint Hash(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

float Random(uint seed)
{
    return (Hash(seed) & 0x00FFFFFF) / 16777216.0f;
}

static float GetRandomSign(uint seed)
{
    return (Random(seed) > 0.5f) ? 1.0f : -1.0f;
}

SamplerState SamplerState0 : register(s0);

//StructuredBuffer<float4x4> DynamicMatrix : register(t0);
//StructuredBuffer<MATERIAL> Materials : register(t1);
//StructuredBuffer<uint> ModelInstanceIds : register(t2);
//StructuredBuffer<uint> ModelInstanceMaterials : register(t3);
//StructuredBuffer<float4x4> NodeMatrix : register(t4);
//StructuredBuffer<MESH_INSTANCE> MeshInstance : register(t5);

