// =======================================================
// animationFollower.h
// 
// 行列をモデルインスタンスの特定のノーツに追従させる
// 「ソケット」のような物
// ※あくまでもGPU上追従させるだけで、CPUへ同期はしない
// =======================================================
#pragma once
#include <vector>
#include <set>
#include <unordered_map>
#include "dataType.h"
#include "dynamicMatrix.h"
#include "modelInstance.h"
#include "buffer.h"

namespace MG {
	class AnimationFollower {
		struct DATA {
			DynamicMatrix dynamicMatrix;
			ModelInstance modelInstance;
			unsigned int nodeIndex;
		};

	private:
		static inline std::vector<DATA> s_Data;
		static inline std::set<unsigned int> s_EmptyIds{};
		static inline ID3D11Buffer* s_Buffer = nullptr;
		static inline ID3D11ShaderResourceView* s_SRV = nullptr;
		static inline unsigned int s_Capcity = 0;
		static inline bool s_NeedUpdateBuffer = false;

	public:
		static size_t GetCount() { return s_Data.size(); }
		static ID3D11ShaderResourceView* const GetSRV() { return s_SRV; }
		static AnimationFollower Create(DynamicMatrix dynamicMatrix, ModelInstance modelInstance, unsigned int nodeIndex)
		{
			AnimationFollower key = {};
			DATA data{};
			data.dynamicMatrix = dynamicMatrix;
			data.modelInstance = modelInstance;
			data.nodeIndex = nodeIndex;

			if (s_EmptyIds.empty()) {
				s_Data.push_back(data);
				key.m_Id = static_cast<unsigned int>(s_Data.size()) - 1;
			}
			else {
				key.m_Id = *s_EmptyIds.begin();
				s_EmptyIds.erase(s_EmptyIds.begin());
				s_Data[key.m_Id] = data;
			}

			s_NeedUpdateBuffer = true;
			return key;
		}

		static void Uninit();
		static void Update();

	private:
		unsigned int m_Id = UINT_MAX;

	public:
		BUFFER_HANDLE_OPERATOR(AnimationFollower)

		const DATA& GetData() const { return s_Data[m_Id]; }

		void SetData(DynamicMatrix dynamicMatrix, ModelInstance modelInstance, unsigned int nodeIndex)
		{
			if (m_Id != UINT_MAX) {
				s_Data[m_Id].dynamicMatrix = dynamicMatrix;
				s_Data[m_Id].modelInstance = modelInstance;
				s_Data[m_Id].nodeIndex = nodeIndex;
				s_NeedUpdateBuffer = true;
			}
		}

		void Release() 
		{
			if (m_Id != UINT_MAX) {
				s_Data[m_Id] = {};

				s_EmptyIds.insert(m_Id);
				m_Id = UINT_MAX;
				s_NeedUpdateBuffer = true;
			}
		}
	};

}