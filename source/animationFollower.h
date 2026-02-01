#pragma once
#include <vector>
#include <set>
#include <unordered_map>
#include "dataType.h"
#include "dynamicMatrix.h"
#include "modelInstance.h"

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;

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
		static void Uninit();
		static void Update();
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

	private:
		unsigned int m_Id = UINT_MAX;
	public:

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

		void Release() {
			if (m_Id != UINT_MAX) {
				s_Data[m_Id] = {};

				s_EmptyIds.insert(m_Id);
				m_Id = UINT_MAX;
				s_NeedUpdateBuffer = true;
			}
		}

		operator bool() const {
			return m_Id != UINT_MAX;
		}

		operator unsigned int() const {
			return m_Id;
		}

		bool operator ==(AnimationFollower& other) const {
			return m_Id == other.m_Id;
		}

		AnimationFollower& operator=(const unsigned int& id) { m_Id = id; }
		AnimationFollower() = default;
		AnimationFollower(const unsigned int& id) :m_Id(id) {}
	};

}