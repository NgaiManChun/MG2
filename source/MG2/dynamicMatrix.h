#pragma once
#include <vector>
#include <set>
#include "matrix4x4.h"

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;

namespace MG {

	class DynamicMatrix {
		
	private:
		static inline std::vector<Matrix4x4> s_Data;
		static inline std::set<unsigned int> s_EmptyIds{};
		
		static inline bool s_NeedUpdateBuffer = false;

		static inline ID3D11Buffer* s_Buffer = nullptr;
		static inline ID3D11ShaderResourceView* s_SRV = nullptr;
		static inline ID3D11UnorderedAccessView* s_UAV = nullptr;
		static inline unsigned int s_Capcity = 0;
		static const constexpr unsigned int DATA_INTERVAL = 30;

	public:
		static void Uninit();
		static void Update();
		static ID3D11Buffer* GetBuffer() { return s_Buffer; }
		static ID3D11ShaderResourceView* const GetSRV() { return s_SRV; }
		static ID3D11UnorderedAccessView* GetUAV() { return s_UAV; }

		static DynamicMatrix Create(const Matrix4x4& data = {}) {
			DynamicMatrix key = {};
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
		void SetData(const Matrix4x4& data) {
			s_Data[m_Id] = data;
			s_NeedUpdateBuffer = true;
		}

		const Matrix4x4& GetData() const { return s_Data[m_Id]; }

		void Release() {
			if (m_Id != UINT_MAX) {
				s_EmptyIds.insert(m_Id);
				m_Id = UINT_MAX;
			}
		}

		operator bool() {
			return m_Id != UINT_MAX;
		}

		operator unsigned int() {
			return m_Id;
		}
	};

}

