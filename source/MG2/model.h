// =======================================================
// model.h
// 
// モデルクラス
// =======================================================
#pragma once
#include <vector>
#include <set>
#include <array>
#include "texture.h"
#include "material.h"
#include "mesh.h"
#include "animation.h"
#include "modelAnimation.h"
#include "matrixDivision.h"
#include "dynamicIndexDivision.h"
#include "buffer.h"

namespace MG {
	class Model {
	public:
		struct DATA {
			unsigned int nodeCount;
			std::unordered_map<std::string, unsigned int> namedNodeIndexes;
			std::vector<Texture> textures;
			std::vector<Material> materials;
			std::vector<Mesh> meshes;
			std::vector<NODE_MESH_PAIR> nodeMeshPairs;
			MatrixDivision originalNodeMatrixDivision;
			DynamicIndexDivision nodeParentIndexDivision;
			std::array<ModelAnimation, UCHAR_MAX> animations;
		};
		
	private:
		static inline std::vector<DATA> s_Data;
		static inline std::unordered_map<std::string, unsigned int> s_NameMap;
		static inline std::set<unsigned int> s_EmptyIds{};

	public:
		static void Uninit();
		static Model Create(const char* filename);

	private:
		unsigned int m_Id = UINT_MAX;

	public:
		BUFFER_HANDLE_OPERATOR(Model)

		DATA& GetData() const { return s_Data[m_Id]; }
		unsigned int GetNodeCount() const { return s_Data[m_Id].nodeCount; }
		ModelAnimation GetModelAnimation(unsigned char slot) const { return s_Data[m_Id].animations[slot]; }

		void Release() 
		{
			if (m_Id != UINT_MAX) {
				DATA& data = s_Data[m_Id];
				for (auto& modelAnimation : data.animations) {
					modelAnimation.Release();
				}
				data.originalNodeMatrixDivision.Release();
				data.nodeParentIndexDivision.Release();
				data = {};

				s_EmptyIds.insert(m_Id);
				m_Id = UINT_MAX;
			}
		}

		// アニメーションをモデル適用する
		void BindAnimation(unsigned char slot, const Animation animation, bool loop = false);

	};

}
