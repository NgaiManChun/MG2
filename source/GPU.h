#pragma once
#include "vector3.h"
#include "vector2.h"
#include "vector4.h"
#include "matrix4x4.h"
#include <vector>

using namespace MG;

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;

class Transform {
	struct TRANSFORM
	{
		Matrix4x4 transform;
	};
private:
	unsigned int m_Id;

	static unsigned int s_Capcity;
	static unsigned int s_Last;
};

class VertexGroup {
	struct VERTEX
	{
		Vector3 position;
		Vector3 normal;
		Vector3 tangent;
		Vector3 bitangent;
		Vector2 texCoord;
		RGBA color;
	};
private:
	unsigned int m_Id;
	unsigned int m_Length;

	static unsigned int s_Capcity;
	static unsigned int s_Last;
};

class VertexIndexGroup {
	struct VERTEX_INDEX
	{
		unsigned int index;
	};
private:
	unsigned int m_Id;
	unsigned int m_Length;

	static unsigned int s_Capcity;
	static unsigned int s_Last;
};

class BoneGroup {
	struct BONE
	{
		Matrix4x4 offset;
		unsigned int nodeOffset;
	};
private:
	unsigned int m_Id;
	unsigned int m_Length;

	static unsigned int s_Capcity;
	static unsigned int s_Last;
};

class VertexBoneWeightGroup {
	struct VERTEX_BONE_WEIGHT
	{
		unsigned int boneOffset[4];
		float weights[4];
	};
private:
	unsigned int m_Id;
	unsigned int m_Length;

	static unsigned int s_Capcity;
	static unsigned int s_Last;
};

class Mesh {
	struct MESH {
		VertexGroup vertexGroup;
		VertexIndexGroup vertexIndexGroup;
		BoneGroup boneGroup;
		VertexBoneWeightGroup vertexBoneWeightGroup;
	};
private:
	unsigned int m_Id;

	static unsigned int s_Capcity;
	static unsigned int s_Last;
};


class ModelNodeGroup {
	struct MODEL_NODE {
		Matrix4x4 transform;
	};
	struct HIERARCHY {
		int nodeOffset;
		int parent;
	};
private:
	unsigned int m_Id;
	unsigned int m_Length;

	static unsigned int s_Capcity;
	static unsigned int s_Last;
};

class ModelNodeMeshGroup {
	struct MODEL_NODE_MESH {
		Mesh mesh;
		unsigned int nodeOffset;
		Vector3 min;
		Vector3 max;
	};
private:
	unsigned int m_Id;
	unsigned int m_Length;

	static unsigned int s_Capcity;
	static unsigned int s_Last;
};

class Model {
	struct MODEL {
		ModelNodeGroup nodeGroup;
		ModelNodeMeshGroup nodeMeshGroup;
	};
private:
	unsigned int m_Id;

	static unsigned int s_Capcity;
	static unsigned int s_Last;
};

class ModelAnimation {
	struct MODEL_ANIMATION {
		Model model;
		unsigned int frameNum;
		unsigned int duration;
		unsigned int loop;
		ModelNodeGroup animatedNodeGroup;
		ModelNodeMeshGroup animatedNodeMeshGroup;
	};
private:
	unsigned int m_Id;

	static unsigned int s_Capcity;
	static unsigned int s_Last;
};

class AnimationSet {
	struct ANIMATION_SET {
		ModelAnimation animations[1];
		unsigned int startTime[1];
	};
private:
	unsigned int m_Id;

	static unsigned int s_Capcity;
	static unsigned int s_Last;
};

class ModelInstance {
	struct MODEL_INSTANCE {
		Model model;
		Transform world;
		AnimationSet animationSet0;
		AnimationSet animationSet1;
		unsigned int animationBlendDuration;
		unsigned int animationBlendStartTime;
		float timeMultiplier;
		bool flags{4}; // 0: enabled, 1: animation
	};
private:
	unsigned int m_Id;

	static unsigned int s_Capcity;
	static unsigned int s_Last;
};

class ModelNodeMeshInstance {
	struct MODEL_NODE_MESH_INSTANCE {
		ModelInstance modelInstance;
		unsigned int nodeMeshOffset;
		//Material material;
	};
private:
	unsigned int m_Id;

	static unsigned int s_Capcity;
	static unsigned int s_Last;
};

struct {
	Mesh instance;
	Matrix4x4 world;
	Matrix4x4 local;
	float z;
};

struct CALL_COUNT {
	unsigned int opaque;
	unsigned int opaqueAnimated;
	unsigned int opaqueSkined;
	unsigned int transparent;
};

/*
ModelNodeMeshInstance
ModelInstance = ModelNodeMeshInstance.modelInstance
Model = model
AnimationSet = ModelInstance.animationSet0
ModelAnimation = AnimationSet.animations[0]
ModelNodeMesh = ModelAnimation.animatedNodeMeshGroup[ModelNodeMeshInstance.nodeMeshOffset]


World = ModelInstance.world;
Local = ModelNodeGroup[ModelNodeMesh.nodeOffset];
AnimatedMin = ModelNodeMesh.min;
AnimatedMax = ModelNodeMesh.max;

Mesh = ModelNodeMesh.mesh;
hasAnimation = ModelInstance.flags[1];
hasSkinning = Mesh.boneGroup.length > 0;

No Animation
	Mesh
	ZSort
Has Animation
	Mesh
	ZSort
SkinAnimation
	Mesh
	ZSort
Transparent
	ZSort





*/


		