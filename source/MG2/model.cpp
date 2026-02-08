#include "model.h"
#include "renderer.h"
#include "MGUtility.h"
#include "texture.h"
#include "material.h"
#include <fstream>

namespace MG {

	Model Model::Create(const char* filename)
	{
		Model key{};

		// すでにロードしたかをチェック
		if (s_NameMap.count(filename) > 0) {
			key.m_Id = s_NameMap.at(filename);
			return key;
		}

		std::ifstream file(filename, std::ios::binary);

		// 頭文字とバージョンをチェック
		char prefix[4];
		file.read(prefix, 4);
		if (strcmp(prefix, MODEL_MAGIC)) {
			file.close();
			return {};
		}
		char version[8];
		file.read(version, 8);

		
		MODEL_META modelMeta{};
		file.read((char*)&modelMeta, sizeof(MODEL_META));

		std::vector<TEXTURE_META> textureMetaArray(modelMeta.textureCount);
		std::vector<MATERIAL_DATA> materialArray(modelMeta.materialCount);
		std::vector<VERTEX> vertexArray(modelMeta.vertexCount);
		std::vector<unsigned int> vertexIndexArray(modelMeta.vertexIndexCount);
		std::vector<BONE> boneArray(modelMeta.boneCount);
		std::vector<VERTEX_BONE_WEIGHT> vertexBoneWeightArray(modelMeta.vertexBoneWeightCount);
		std::vector<MESH_DATA> meshDataArray(modelMeta.meshCount);
		std::vector<Matrix4x4> nodeMatrixArray(modelMeta.nodeCount);
		std::vector<unsigned int> nodeParentArray(modelMeta.nodeCount);
		std::vector<char> nameArray(modelMeta.nodeNameSize);

		DATA data{};
		data.nodeCount = modelMeta.nodeCount;
		data.nodeMeshPairs.resize(modelMeta.nodeMeshPairCount);
		
		// テクスチャ読み込み
		file.read((char*)textureMetaArray.data(), sizeof(TEXTURE_META) * modelMeta.textureCount);
		std::vector<unsigned char> dataBuffer;
		for (unsigned int i = 0; i < modelMeta.textureCount; i++) {
			TEXTURE_META& meta = textureMetaArray[i];

			// バッファリサイズ
			if (meta.length > dataBuffer.size()) {
				dataBuffer.resize(meta.length);
			}

			std::string name = std::string(filename) + std::string("::t") + std::to_string(i);

			file.read((char*)dataBuffer.data(), sizeof(unsigned char) * meta.length);
			data.textures.push_back(
				Texture::Create(name.c_str(), dataBuffer.data(), sizeof(unsigned char) * meta.length)
			);
		}

		// データ読み込み
		file.read((char*)materialArray.data(), sizeof(MATERIAL_DATA) * modelMeta.materialCount);
		file.read((char*)vertexArray.data(), sizeof(VERTEX) * modelMeta.vertexCount);
		file.read((char*)vertexIndexArray.data(), sizeof(unsigned int) * modelMeta.vertexIndexCount);
		file.read((char*)boneArray.data(), sizeof(BONE) * modelMeta.boneCount);
		file.read((char*)vertexBoneWeightArray.data(), sizeof(VERTEX_BONE_WEIGHT) * modelMeta.vertexBoneWeightCount);
		file.read((char*)meshDataArray.data(), sizeof(MESH_DATA) * modelMeta.meshCount);
		file.read((char*)nodeMatrixArray.data(), sizeof(Matrix4x4) * modelMeta.nodeCount);
		file.read((char*)nodeParentArray.data(), sizeof(unsigned int) * modelMeta.nodeCount);
		file.read((char*)data.nodeMeshPairs.data(), sizeof(NODE_MESH_PAIR) * modelMeta.nodeMeshPairCount);
		file.read((char*)nameArray.data(), sizeof(char) * modelMeta.nodeNameSize);
		file.close();

		// ヒエラルキー行列作成
		data.originalNodeMatrixDivision = MatrixDivision::Create(
			modelMeta.nodeCount, nodeMatrixArray.data()
		);

		// ヒエラルキー親インデックスバッファ作成
		data.nodeParentIndexDivision = DynamicIndexDivision::Create(modelMeta.nodeCount, nodeParentArray.data());

		// ノーツの名前を設定する
		size_t nameOffset = 0;
		for (unsigned int i = 0; i < modelMeta.nodeCount; i++) {
			const char* name = (nameArray.data() + nameOffset);
			data.namedNodeIndexes[name] = i;
			nameOffset += strlen(name) + 1;
		}

		// マテリアル作成
		for (MATERIAL_DATA& materialData : materialArray) {
			data.materials.push_back(Material::Create({
				materialData.base,
				materialData.emissive,
				materialData.specular,
				materialData.metallic,
				materialData.roughness,
				materialData.shininess,
				(materialData.baseTexture >= 0) ? data.textures[materialData.baseTexture] : Texture{},
				(materialData.normalTexture >= 0) ? data.textures[materialData.normalTexture] : Texture{},
				(materialData.opacityTexture >= 0) ? data.textures[materialData.opacityTexture] : Texture{},
				materialData.type,
				materialData.opaque
			}));
		}

		// メッシュ作成
		for (MESH_DATA& meshData : meshDataArray) {
			data.meshes.push_back(Mesh::Create({
				(vertexArray.data() + meshData.vertexesOffset),
				(meshData.boneCount > 0) ? (vertexBoneWeightArray.data() + meshData.vertexBoneWeightsOffset) : nullptr,
				meshData.vertexCount,
				(vertexIndexArray.data() + meshData.vertexIndexesOffset),
				meshData.vertexIndexCount,
				(boneArray.data() + meshData.bonesOffset),
				meshData.boneCount,
				meshData.materialOffset,
				meshData.primitiveType,
				meshData.min,
				meshData.max
			}));
		}

		if (s_EmptyIds.empty()) {
			s_Data.push_back(data);
			key.m_Id = s_Data.size() - 1;
		}
		else {
			key.m_Id = *s_EmptyIds.begin();
			s_EmptyIds.erase(s_EmptyIds.begin());
			s_Data[key.m_Id] = data;
		}

		s_NameMap[filename] = key.m_Id;
		return key;
	}

	void Model::Uninit()
	{
		for (auto& data : s_Data) {
			for (auto& texture : data.textures) {
				texture.Release();
			}
			for (auto& material : data.materials) {
				material.Release();
			}
			for (auto& mesh : data.meshes) {
				mesh.Release();
			}
			for (auto& animation : data.animations) {
				animation.Release();
			}
			data.originalNodeMatrixDivision.Release();
			data.nodeParentIndexDivision.Release();
		}
		s_Data.clear();
		s_EmptyIds.clear();
	}

	void Model::BindAnimation(unsigned char slot, const Animation animation, bool loop)
	{
		s_Data[m_Id].animations[slot].Release();
		s_Data[m_Id].animations[slot] = ModelAnimation::Create(m_Id, animation, loop);
	}

} // namespace MG
