#pragma once
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"
#include "matrix4x4.h"

namespace MG {

	enum MATERIAL_TYPE {
		MATERIAL_TYPE_FLAT,
		MATERIAL_TYPE_GOURAUD,
		MATERIAL_TYPE_PHONG,
		MATERIAL_TYPE_BLINN,
		MATERIAL_TYPE_TOON,
		MATERIAL_TYPE_OREN_NAYAR,
		MATERIAL_TYPE_MINNAERT,
		MATERIAL_TYPE_COOK_TORRANCE,
		MATERIAL_TYPE_UNLIT,
		MATERIAL_TYPE_FRESNEL,
		MATERIAL_TYPE_PBR_BRDF,
		MATERIAL_TYPE_OTHER
	};

	enum PRIMITIVE_TYPE {
		PRIMITIVE_TYPE_POINT,
		PRIMITIVE_TYPE_LINE,
		PRIMITIVE_TYPE_TRIANGLE,
		PRIMITIVE_TYPE_NONE
	};

	enum LOD {
		LOD_0 = 0x1,
		LOD_1 = 0x2,
		LOD_2 = 0x4,
		LOD_3 = 0x8,
		LOD_4 = 0x10,
		LOD_ALL = 0x1F
	};

	struct TRANSFORM
	{
		Vector3 position{ 0.0f, 0.0f, 0.0f };
		Vector3 scale{ 1.0f, 1.0f, 1.0f };
		Quaternion rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
	};

	static constexpr const char MODEL_MAGIC[4] = "MGM";
	static constexpr const char MODEL_VERSION[8] = "1.0";
	static constexpr const char ANIMATION_MAGIC[4] = "MGA";
	static constexpr const char ANIMATION_VERSION[8] = "1.0";
	struct MODEL_META {
		unsigned int textureCount;
		unsigned int materialCount;
		unsigned int vertexCount;
		unsigned int vertexIndexCount;
		unsigned int boneCount;
		unsigned int vertexBoneWeightCount;
		unsigned int meshCount;
		unsigned int nodeCount;
		unsigned int nodeMeshPairCount;
		size_t nodeNameSize;
		size_t dataSize;
	};

	struct TEXTURE_META
	{
		size_t length;
		size_t type;
	};

	struct MATERIAL_DATA
	{
		Vector4 base;
		Vector4 emissive;
		Vector4 specular;
		float metallic;
		float roughness;
		float shininess;
		int baseTexture;
		int normalTexture;
		int opacityTexture;
		bool opaque;
		MATERIAL_TYPE type;
	};

	struct VERTEX
	{
		Vector3 position;
		Vector3 normal;
		Vector3 tangent;
		Vector3 bitangent;
		Vector2 texCoord;
		RGBA color;
	};

	struct BONE
	{
		Matrix4x4 offsetMatrix;
		unsigned int nodeIndex;
	};

	struct VERTEX_BONE_WEIGHT
	{
		unsigned int boneId[4];
		float weights[4];
	};

	struct MESH_DATA {
		PRIMITIVE_TYPE primitiveType;

		unsigned int vertexesOffset;
		unsigned int vertexCount;

		unsigned int vertexIndexesOffset;
		unsigned int vertexIndexCount;

		unsigned int bonesOffset;
		unsigned int boneCount;

		unsigned int vertexBoneWeightsOffset;

		unsigned int materialOffset;

		Vector3 min;
		Vector3 max;
	};

	struct NODE_MESH_PAIR {
		unsigned int nodeOffset;
		unsigned int meshOffset;
	};

	struct MESH_DESC {
		VERTEX* vertexes;
		VERTEX_BONE_WEIGHT* vertexBoneWeights;
		unsigned int vertexCount;

		unsigned int* vertexIndexes;
		unsigned int vertexIndexCount;

		BONE* bones;
		unsigned int boneCount;

		unsigned int materialOffset;
		PRIMITIVE_TYPE primitiveType;
		Vector3 min;
		Vector3 max;
	};

	struct ANIMATION_KEY_VECTOR {
		unsigned int frame;
		Vector3 value;
	};

	struct ANIMATION_KEY_QUATERNION {
		unsigned int frame;
		Quaternion value;
	};

	struct ANIMATION_CHANNEL {
		unsigned int positionKeyOffset;
		unsigned int positionKeyCount;

		unsigned int scalingKeyOffset;
		unsigned int scalingKeyCount;

		unsigned int rotationKeyOffset;
		unsigned int rotationKeyCount;
		
	};

	struct ANIMATION_META {
		unsigned int frameRate;
		unsigned int frames;

		unsigned int positionKeyCount;
		unsigned int scalingKeyCount;
		unsigned int rotationKeyCount;
		unsigned int channelCount;
		size_t channelNameLength;
		size_t dataSize;
	};

	struct DRAW_INDEXED_INDIRECT_ARGS {
		unsigned int indexCountPerInstance;
		unsigned int instanceCount;
		unsigned int startIndexLocation;
		int  baseVertexLocation;
		unsigned int startInstanceLocation;
		unsigned int instanceMaxCount;
	};

	struct DRAW_INDIRECT_ARGS {
		unsigned int vertexCountPerInstance;
		unsigned int instanceCount;
		unsigned int startVertexLocation;
		unsigned int startInstanceLocation;
		unsigned int instanceMaxCount;
	};

	struct PARTICLE
	{
		TRANSFORM transform;
		Vector3 velocity;
		Vector3 acceleration;
		RGBA color;
		float fade;
		float life;
	};

	struct AABB
	{
		Vector3 min;
		Vector3 max;
	};

	struct BOOKMARK {
		unsigned int offset;
		unsigned int count;
		unsigned int padding;
	};

}; // namespace MG
