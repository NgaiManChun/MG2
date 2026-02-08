// =======================================================
// mesh.h
// 
// メッシュのバッファ
// =======================================================
#pragma once
#include <vector>
#include <set>
#include "vertexDivision.h"
#include "vertexIndexDivision.h"
#include "boneDivision.h"
#include "vertexBoneWeightDivision.h"
#include "buffer.h"

namespace MG {
	class Mesh {
	public:
		struct DATA {
			VertexDivision vertexDivision;
			VertexIndexDivision vertexIndexDivision;
			unsigned int materialOffset;
			PRIMITIVE_TYPE primitiveType;
			Vector3 min;
			Vector3 max;
			BoneDivision boneDivision;
			VertexBoneWeightDivision vertexBoneWeightDivision;
		};
		
	private:
		static inline std::vector<DATA> s_Data;
		static inline std::vector<DRAW_INDEXED_INDIRECT_ARGS> s_DrawArgs;
		static inline std::set<unsigned int> s_EmptyIds{};
		static inline ID3D11Buffer* s_DrawArgsBuffer = nullptr;
		static inline ID3D11UnorderedAccessView* s_DrawArgsUAV = nullptr;
		static inline ID3D11ShaderResourceView* s_DrawArgsSRV = nullptr;
		static inline ID3D11Buffer* s_DrawArgsIndirectBuffer = nullptr;
		static inline unsigned int s_DrawArgsBufferCapcity = 0;
		static inline bool s_NeedUpdateBuffer = false;

	public:
		static ID3D11Buffer* GetDrawArgsBuffer() { return s_DrawArgsBuffer; }
		static ID3D11UnorderedAccessView* GetDrawArgsUAV() { return s_DrawArgsUAV; }
		static ID3D11ShaderResourceView* GetDrawArgsSRV() { return s_DrawArgsSRV; }
		static ID3D11Buffer* GetDrawArgsIndirectBuffer() { return s_DrawArgsIndirectBuffer; }
		static unsigned int GetDrawArgsCount() { return static_cast<unsigned int>(s_DrawArgs.size()); }
		
		static Mesh Create(const MESH_DESC& meshDesc) {
			Mesh key = {};

			DATA data{};
			data.vertexDivision = VertexDivision::Create(meshDesc.vertexCount, meshDesc.vertexes);
			data.vertexIndexDivision = VertexIndexDivision::Create(meshDesc.vertexIndexCount, meshDesc.vertexIndexes);
			data.primitiveType = meshDesc.primitiveType;
			data.materialOffset = meshDesc.materialOffset;
			data.min = meshDesc.min;
			data.max = meshDesc.max;
			if (meshDesc.boneCount > 0) {
				data.boneDivision = BoneDivision::Create(meshDesc.boneCount, meshDesc.bones);
				data.vertexBoneWeightDivision = VertexBoneWeightDivision::Create(meshDesc.vertexCount, meshDesc.vertexBoneWeights);
			}

			// メッシュ分のIndirectArgsを用意する
			DRAW_INDEXED_INDIRECT_ARGS drawArgs{};
			drawArgs.indexCountPerInstance = meshDesc.vertexIndexCount;
			drawArgs.instanceCount = 0;
			drawArgs.startIndexLocation = data.vertexIndexDivision.GetBookmarkData().offset;
			drawArgs.baseVertexLocation = data.vertexDivision.GetBookmarkData().offset;
			drawArgs.startInstanceLocation = 0;

			if (s_EmptyIds.empty()) {
				s_Data.push_back(data);
				s_DrawArgs.push_back(drawArgs);
				key.m_Id = static_cast<unsigned int>(s_Data.size()) - 1;
			}
			else {
				key.m_Id = *s_EmptyIds.begin();
				s_EmptyIds.erase(s_EmptyIds.begin());
				s_Data[key.m_Id] = data;
				s_DrawArgs[key.m_Id] = drawArgs;
			}
			s_NeedUpdateBuffer = true;
			return key;
		}

		static void Update();
		static void Uninit();

	private:
		unsigned int m_Id = UINT_MAX;

	public:
		BUFFER_HANDLE_OPERATOR(Mesh)

		const DATA& GetData() const { return s_Data[m_Id]; }
		const DRAW_INDEXED_INDIRECT_ARGS& GetArgs() const { return s_DrawArgs[m_Id]; }

		void Release() 
		{
			if (m_Id != UINT_MAX) {
				s_Data[m_Id].vertexDivision.Release();
				s_Data[m_Id].vertexIndexDivision.Release();
				s_Data[m_Id].boneDivision.Release();
				s_Data[m_Id].vertexBoneWeightDivision.Release();
				s_Data[m_Id] = {};
				s_DrawArgs[m_Id] = {};

				s_EmptyIds.insert(m_Id);
				m_Id = UINT_MAX;
			}
		}
	};

}

