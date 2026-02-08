#include "model.h"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/matrix4x4.h"
#pragma comment (lib, "assimp-vc143-mt.lib")

#include "dataType.h"
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"
#include "matrix4x4.h"

#include <vector>
#include <unordered_map>
#include <fstream>
#include <iostream>

using namespace MG;

struct NODE {
	Matrix4x4 matrix;
	unsigned int parent;
	std::string name;
	std::vector<unsigned int> meshes;
};

MATERIAL_TYPE GetMaterialType(aiShadingMode shadingModel)
{
	static std::unordered_map<aiShadingMode, MATERIAL_TYPE> map{
		{ aiShadingMode_Flat, MATERIAL_TYPE_FLAT },
		{ aiShadingMode_Gouraud, MATERIAL_TYPE_GOURAUD },
		{ aiShadingMode_Phong, MATERIAL_TYPE_PHONG },
		{ aiShadingMode_Blinn, MATERIAL_TYPE_BLINN },
		{ aiShadingMode_Toon, MATERIAL_TYPE_TOON},
		{ aiShadingMode_OrenNayar, MATERIAL_TYPE_OREN_NAYAR },
		{ aiShadingMode_Minnaert, MATERIAL_TYPE_MINNAERT },
		{ aiShadingMode_CookTorrance, MATERIAL_TYPE_COOK_TORRANCE },
		{ aiShadingMode_Unlit, MATERIAL_TYPE_UNLIT },
		{ aiShadingMode_Fresnel, MATERIAL_TYPE_FRESNEL },
		{ aiShadingMode_PBR_BRDF, MATERIAL_TYPE_PBR_BRDF }
	};

	if (map.count(shadingModel) > 0) {
		return map.at(shadingModel);
	}
	return MATERIAL_TYPE_OTHER;
}

PRIMITIVE_TYPE GetPrimitiveType(unsigned int type) {
	static std::unordered_map<unsigned int, PRIMITIVE_TYPE> map{
		{aiPrimitiveType_POINT, PRIMITIVE_TYPE_POINT},
		{aiPrimitiveType_LINE, PRIMITIVE_TYPE_LINE},
		{aiPrimitiveType_TRIANGLE, PRIMITIVE_TYPE_TRIANGLE}
	};
	if (map.count(type) > 0) {
		return map[type];
	}
	return PRIMITIVE_TYPE_NONE;
}

unsigned int GetFaceIndexCount(PRIMITIVE_TYPE type)
{
	static std::unordered_map<PRIMITIVE_TYPE, unsigned int> map{
		{PRIMITIVE_TYPE_TRIANGLE, 3},
		{PRIMITIVE_TYPE_LINE, 2},
		{PRIMITIVE_TYPE_POINT, 1}
	};
	if (map.count(type) > 0) {
		return map[type];
	}
	return 0;
}

void GetNodes(aiNode* ainode, unsigned int parent, std::vector<NODE>& nodes)
{
	NODE node{};
	node.parent = parent;
	node.name = ainode->mName.C_Str();
	for (unsigned int i = 0; i < ainode->mNumMeshes; i++) {
		node.meshes.push_back(ainode->mMeshes[i]);
	}
	aiMatrix4x4& matrix = ainode->mTransformation;
	node.matrix = {
		matrix.a1, matrix.a2, matrix.a3, matrix.a4,
		matrix.b1, matrix.b2, matrix.b3, matrix.b4,
		matrix.c1, matrix.c2, matrix.c3, matrix.c4,
		matrix.d1, matrix.d2, matrix.d3, matrix.d4
	};
	nodes.push_back(node);
	unsigned int currentIndex = nodes.size() - 1;
	for (unsigned int i = 0; i < ainode->mNumChildren; i++) {
		GetNodes(ainode->mChildren[i], currentIndex, nodes);
	}
}

void ReadModel(const char* fileName, const char* outputName)
{
	const aiScene* scene = aiImportFile(fileName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded | aiProcess_GenBoundingBoxes);
	assert(scene);

	std::vector<TEXTURE_META> textureMetaArray;
	std::vector<unsigned char> textureDataArray;
	std::vector<MATERIAL_DATA> materialArray;
	std::vector<VERTEX> vertexArray;
	std::vector<unsigned int> vertexIndexArray;
	std::vector<BONE> boneArray;
	std::vector<VERTEX_BONE_WEIGHT> vertexBoneWeightArray;
	std::vector<MESH_DATA> meshArray;
	std::vector<Matrix4x4> nodeMatrixArray;
	std::vector<unsigned int> nodeParentArray;
	std::vector<char> nodeNameArray;
	std::vector<NODE_MESH_PAIR> nodeMeshPairArray;
	std::unordered_map<std::string, unsigned int> namedNodeIndex;

	// ノード読み込み
	{
		std::vector<NODE> nodes;
		GetNodes(scene->mRootNode, UINT_MAX, nodes);

		unsigned int nodeCount = nodes.size();
		nodeMatrixArray.resize(nodeCount);
		nodeParentArray.resize(nodeCount);
		unsigned int nameLength = 0;
		for (unsigned int i = 0; i < nodes.size(); i++) {
			NODE& node = nodes[i];
			nameLength += node.name.size() + 1;
			nodeMatrixArray[i] = node.matrix;
			nodeParentArray[i] = node.parent;

			for (unsigned int meshId : node.meshes) {
				nodeMeshPairArray.push_back({
					i,
					meshId
				});
			}
			namedNodeIndex[node.name] = i;
		}
		nodeNameArray.resize(nameLength);
		unsigned int offset = 0;
		for (unsigned int i = 0; i < nodes.size(); i++) {
			NODE& node = nodes[i];
			memcpy(nodeNameArray.data() + offset, node.name.data(), node.name.size());
			offset += node.name.size();
			nodeNameArray[offset++] = '\0';
		}
	}

	for (unsigned int i = 0; i < nodeMatrixArray.size(); i++) {
		Matrix4x4& matrix = nodeMatrixArray[i];
		unsigned int parent = nodeParentArray[i];
		if (parent != UINT_MAX) {
			matrix = nodeMatrixArray[parent] * matrix;
		}
	}

	std::sort(nodeMeshPairArray.begin(), nodeMeshPairArray.end(), [](NODE_MESH_PAIR& a, NODE_MESH_PAIR& b) {
		return a.meshOffset < b.meshOffset;
	});
	

	// テクスチャ読み込み
	{
		std::vector<unsigned char*> textureDataPointers;
		size_t textureDataLength = 0;
		textureMetaArray.resize(scene->mNumTextures);
		textureDataPointers.resize(scene->mNumTextures);
		for (size_t i = 0; i < scene->mNumTextures; i++)
		{
			aiTexture* aitexture = scene->mTextures[i];
			TEXTURE_META& textureMeta = textureMetaArray[i];
			textureMeta.type = 0;
			textureMeta.length = (aitexture->mHeight == 0) ? aitexture->mWidth : aitexture->mWidth * aitexture->mHeight;
			textureMetaArray[i] = textureMeta;
			textureDataPointers[i] = (unsigned char*)aitexture->pcData;
			textureDataLength += textureMeta.length;
		}

		size_t offset = 0;
		textureDataArray.resize(textureDataLength);
		for (size_t i = 0; i < scene->mNumTextures; i++)
		{
			memcpy(textureDataArray.data() + offset, textureDataPointers[i], textureMetaArray[i].length);
			offset += textureMetaArray[i].length;
		}
	}

	// マテリアル読み込み
	{
		materialArray.resize(scene->mNumMaterials);
		for (size_t i = 0; i < scene->mNumMaterials; i++)
		{
			aiMaterial* aimaterial = scene->mMaterials[i];
			MATERIAL_DATA& material = materialArray[i];
			aiShadingMode shadingModel;
			aiColor3D color3D;
			aiColor4D color4D;
			float factor;
			aiString str;

			if (aimaterial->Get(AI_MATKEY_SHADING_MODEL, shadingModel) == AI_SUCCESS) {
				material.type = GetMaterialType(shadingModel);
			}

			if (aimaterial->Get(AI_MATKEY_BASE_COLOR, color3D) == AI_SUCCESS) {
				material.base = RGBA{ color3D.r, color3D.g, color3D.b };
			}
			else if (aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color3D) == AI_SUCCESS) {
				material.base = RGBA{ color3D.r, color3D.g, color3D.b };
			}

			if (aimaterial->Get(AI_MATKEY_OPACITY, factor) == AI_SUCCESS) {
				material.base.a = factor;
			}
			if (aimaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color4D) == AI_SUCCESS) {
				material.emissive = RGBA{ color4D.r, color4D.g, color4D.b, color4D.a };
			}
			if (aimaterial->Get(AI_MATKEY_METALLIC_FACTOR, factor) == AI_SUCCESS) {
				material.metallic = factor;
			}
			if (aimaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, factor) == AI_SUCCESS) {
				material.roughness = factor;
			}
			if (aimaterial->Get(AI_MATKEY_SHININESS, factor) == AI_SUCCESS) {
				material.shininess = factor;
			}
			if (aimaterial->Get(AI_MATKEY_COLOR_SPECULAR, color4D) == AI_SUCCESS) {
				material.specular = RGBA{ color4D.r, color4D.g, color4D.b, color4D.a };
			}

			material.baseTexture = -1;
			material.normalTexture = -1;
			material.opacityTexture = -1;

			if (aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &str) == AI_SUCCESS) {
				for (unsigned int i = 0; i < scene->mNumTextures; i++)
				{
					aiTexture* aitexture = scene->mTextures[i];
					if (aitexture->mFilename == str) {
						material.baseTexture = i;
						break;
					}
				}
			}
			if (aimaterial->GetTexture(aiTextureType_NORMALS, 0, &str) == AI_SUCCESS) {
				for (unsigned int i = 0; i < scene->mNumTextures; i++)
				{
					aiTexture* aitexture = scene->mTextures[i];
					if (aitexture->mFilename == str) {
						material.normalTexture = i;
						break;
					}
				}
			}
			if (aimaterial->GetTexture(aiTextureType_OPACITY, 0, &str) == AI_SUCCESS) {
				for (unsigned int i = 0; i < scene->mNumTextures; i++)
				{
					aiTexture* aitexture = scene->mTextures[i];
					if (aitexture->mFilename == str) {
						material.opacityTexture = i;
						break;
					}
				}
			}

			material.opaque = (material.base.a < 1.0f || (material.opacityTexture != -1 && material.opacityTexture != material.baseTexture)) ? false : true;
		}
	}

	// メッシュ読み込み
	{
		unsigned int vertexesOffset = 0;
		unsigned int vertexIndexesOffset = 0;
		unsigned int bonesOffset = 0;
		unsigned int vertexBoneWeightsOffset = 0;

		meshArray.resize(scene->mNumMeshes);
		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{
			MESH_DATA& mesh = meshArray[i];
			aiMesh* aimesh = scene->mMeshes[i];

			mesh.primitiveType = GetPrimitiveType(aimesh->mPrimitiveTypes);

			aiVector3D aiMax = aimesh->mAABB.mMax;
			aiVector3D aiMin = aimesh->mAABB.mMin;
			mesh.min.x = aiMin.x;
			mesh.min.y = aiMin.y;
			mesh.min.z = aiMin.z;
			mesh.max.x = aiMax.x;
			mesh.max.y = aiMax.y;
			mesh.max.z = aiMax.z;

			mesh.materialOffset = aimesh->mMaterialIndex;

			mesh.vertexCount = aimesh->mNumVertices;
			mesh.vertexesOffset = vertexesOffset;

			mesh.vertexIndexCount = aimesh->mNumFaces * GetFaceIndexCount(mesh.primitiveType);
			mesh.vertexIndexesOffset = vertexIndexesOffset;

			mesh.boneCount = aimesh->mNumBones;
			mesh.bonesOffset = bonesOffset;

			mesh.vertexBoneWeightsOffset = vertexBoneWeightsOffset;

			vertexesOffset += mesh.vertexCount;
			vertexIndexesOffset += mesh.vertexIndexCount;
			bonesOffset += mesh.boneCount;
			if (mesh.boneCount > 0) {
				vertexBoneWeightsOffset += mesh.vertexCount;
			}
		}

		vertexArray.resize(vertexesOffset);
		vertexIndexArray.resize(vertexIndexesOffset);
		boneArray.resize(bonesOffset);
		vertexBoneWeightArray.resize(vertexBoneWeightsOffset);

		for (size_t i = 0; i < scene->mNumMeshes; i++)
		{
			MESH_DATA& mesh = meshArray[i];
			aiMesh* aimesh = scene->mMeshes[i];

			for (unsigned int v = 0; v < aimesh->mNumVertices; v++)
			{
				VERTEX& vertex = vertexArray[mesh.vertexesOffset + v];
				vertex.position = Vector3{ aimesh->mVertices[v].x, aimesh->mVertices[v].y, aimesh->mVertices[v].z };

				// UVマップ
				if (aimesh->mTextureCoords[0]) {
					vertex.texCoord = { aimesh->mTextureCoords[0][v].x, aimesh->mTextureCoords[0][v].y };
				}
				else {
					vertex.texCoord = { };
				}

				// 法線
				if (aimesh->mNormals) {
					vertex.normal = Vector3{ aimesh->mNormals[v].x, aimesh->mNormals[v].y, aimesh->mNormals[v].z };
				}
				else {
					vertex.normal = Vector3{ };
				}

				// Tangents
				if (aimesh->mTangents) {
					vertex.tangent = Vector3{ aimesh->mTangents[v].x, aimesh->mTangents[v].y, aimesh->mTangents[v].z };
				}
				else {
					vertex.tangent = Vector3{ };
				}

				// Bitangent
				if (aimesh->mBitangents) {
					vertex.bitangent = Vector3{ aimesh->mBitangents[v].x, aimesh->mBitangents[v].y, aimesh->mBitangents[v].z };
				}
				else {
					vertex.bitangent = Vector3{ };
				}

				// 頂点カラー
				if (aimesh->HasVertexColors(0)) {
					vertex.color = RGBA{ aimesh->mColors[0]->r, aimesh->mColors[0]->g, aimesh->mColors[0]->b, aimesh->mColors[0]->a };
				}
				else {
					vertex.color = RGBA{ 1.0f, 1.0f, 1.0f, 1.0f };
				}
			}

			// インデックス
			{
				unsigned int faceIndexNum = GetFaceIndexCount(mesh.primitiveType);
				for (unsigned int f = 0; f < aimesh->mNumFaces; f++)
				{
					const aiFace* face = &aimesh->mFaces[f];
					for (int fi = 0; fi < faceIndexNum; fi++) {
						vertexIndexArray[mesh.vertexIndexesOffset + f * faceIndexNum + fi] = face->mIndices[fi];
					}
				}
			}

			struct BONE_WEIGHT {
				unsigned int boneId = 0;
				float weight = 0.0f;
			};
			std::vector<std::vector<BONE_WEIGHT>> vertexWeights(aimesh->mNumVertices);

			// ボーン情報
			if (aimesh->mNumBones > 0) {

				for (unsigned int b = 0; b < aimesh->mNumBones; b++) {
					boneArray[mesh.bonesOffset + b] = {
						{
							aimesh->mBones[b]->mOffsetMatrix.a1, aimesh->mBones[b]->mOffsetMatrix.a2, aimesh->mBones[b]->mOffsetMatrix.a3, aimesh->mBones[b]->mOffsetMatrix.a4,
							aimesh->mBones[b]->mOffsetMatrix.b1, aimesh->mBones[b]->mOffsetMatrix.b2, aimesh->mBones[b]->mOffsetMatrix.b3, aimesh->mBones[b]->mOffsetMatrix.b4,
							aimesh->mBones[b]->mOffsetMatrix.c1, aimesh->mBones[b]->mOffsetMatrix.c2, aimesh->mBones[b]->mOffsetMatrix.c3, aimesh->mBones[b]->mOffsetMatrix.c4,
							aimesh->mBones[b]->mOffsetMatrix.d1, aimesh->mBones[b]->mOffsetMatrix.d2, aimesh->mBones[b]->mOffsetMatrix.d3, aimesh->mBones[b]->mOffsetMatrix.d4
						},
						namedNodeIndex[aimesh->mBones[b]->mName.C_Str()]
					};

					for (unsigned int w = 0; w < aimesh->mBones[b]->mNumWeights; w++) {
						unsigned int boneId = b;
						unsigned int vertexId = aimesh->mBones[b]->mWeights[w].mVertexId;
						float weight = aimesh->mBones[b]->mWeights[w].mWeight;
						vertexWeights[vertexId].push_back({ boneId , weight });
					}
				}

				for (unsigned int v = 0; v < aimesh->mNumVertices; v++) {
					for (unsigned int w = 0; w < vertexWeights[v].size() && w < 4; w++) {
						unsigned int boneId = vertexWeights[v][w].boneId;
						float weight = vertexWeights[v][w].weight;
						vertexBoneWeightArray[mesh.vertexBoneWeightsOffset + v].boneId[w] = boneId;
						vertexBoneWeightArray[mesh.vertexBoneWeightsOffset + v].weights[w] = weight;
					}
				}
			}
		}
	}

	MODEL_META modelMeta{};
	modelMeta.textureCount = textureMetaArray.size();
	modelMeta.materialCount = materialArray.size();
	modelMeta.vertexCount = vertexArray.size();
	modelMeta.vertexIndexCount = vertexIndexArray.size();
	modelMeta.boneCount = boneArray.size();
	modelMeta.vertexBoneWeightCount = vertexBoneWeightArray.size();
	modelMeta.meshCount = meshArray.size();
	modelMeta.nodeCount = nodeMatrixArray.size();
	modelMeta.nodeMeshPairCount = nodeMeshPairArray.size();
	modelMeta.nodeNameSize = sizeof(char) * nodeNameArray.size();

	modelMeta.dataSize = sizeof(TEXTURE_META) * textureMetaArray.size();
	modelMeta.dataSize += sizeof(unsigned char) * textureDataArray.size();
	modelMeta.dataSize += sizeof(MATERIAL_DATA) * materialArray.size();
	modelMeta.dataSize += sizeof(VERTEX) * vertexArray.size();
	modelMeta.dataSize += sizeof(unsigned int) * vertexIndexArray.size();
	modelMeta.dataSize += sizeof(BONE) * boneArray.size();
	modelMeta.dataSize += sizeof(VERTEX_BONE_WEIGHT) * vertexBoneWeightArray.size();
	modelMeta.dataSize += sizeof(MESH_DATA) * meshArray.size();
	modelMeta.dataSize += sizeof(Matrix4x4) * nodeMatrixArray.size();
	modelMeta.dataSize += sizeof(unsigned int) * nodeParentArray.size();
	modelMeta.dataSize += sizeof(NODE_MESH_PAIR) * nodeMeshPairArray.size();
	modelMeta.dataSize += sizeof(char) * nodeNameArray.size();


	std::string ouputFileName;
	if (outputName) {
		ouputFileName = outputName;
	}
	else {
		ouputFileName = fileName;
		ouputFileName += ".mgm";
	}
	std::ofstream file(ouputFileName, std::ios::binary);

	file.write((const char*)MODEL_MAGIC, sizeof(char) * 4);
	file.write((const char*)MODEL_VERSION, sizeof(char) * 8);
	file.write((const char*)&modelMeta, sizeof(MODEL_META));
	file.write((const char*)textureMetaArray.data(), sizeof(TEXTURE_META) * textureMetaArray.size());
	file.write((const char*)textureDataArray.data(), sizeof(unsigned char) * textureDataArray.size());
	file.write((const char*)materialArray.data(), sizeof(MATERIAL_DATA) * materialArray.size());
	file.write((const char*)vertexArray.data(), sizeof(VERTEX) * vertexArray.size());
	file.write((const char*)vertexIndexArray.data(), sizeof(unsigned int) * vertexIndexArray.size());
	file.write((const char*)boneArray.data(), sizeof(BONE) * boneArray.size());
	file.write((const char*)vertexBoneWeightArray.data(), sizeof(VERTEX_BONE_WEIGHT) * vertexBoneWeightArray.size());
	file.write((const char*)meshArray.data(), sizeof(MESH_DATA) * meshArray.size());
	file.write((const char*)nodeMatrixArray.data(), sizeof(Matrix4x4) * nodeMatrixArray.size());
	file.write((const char*)nodeParentArray.data(), sizeof(unsigned int) * nodeParentArray.size());
	file.write((const char*)nodeMeshPairArray.data(), sizeof(NODE_MESH_PAIR) * nodeMeshPairArray.size());
	file.write((const char*)nodeNameArray.data(), sizeof(char) * nodeNameArray.size());
	file.close();

	aiReleaseImport(scene);
}