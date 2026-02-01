#pragma once
#include <vector>
#include <set>
#include "matrix4x4.h"

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;
struct ID3D11ComputeShader;

namespace MG {
	class DynamicIndexDivision {
		struct META {
			unsigned int offset;
			unsigned int count;
			unsigned int padding;
		};
	private:
		static inline std::vector<META> s_Meta{};
		static inline std::vector<unsigned int> s_Data{};
		static inline std::set<unsigned int> s_EmptyIds{};

		static inline ID3D11Buffer* s_MetaBuffer = nullptr;
		static inline ID3D11ShaderResourceView* s_MetaSRV = nullptr;
		static inline ID3D11Buffer* s_DataBuffer = nullptr;
		static inline ID3D11ShaderResourceView* s_DataSRV = nullptr;
		static inline unsigned int s_MetaCapcity = 0;
		static inline unsigned int s_DataCapcity = 0;
		static inline bool s_NeedUpdateBuffer = false;

	public:
		static ID3D11Buffer* const GetMetaBuffer() { return s_MetaBuffer; }
		static ID3D11ShaderResourceView* const GetMetaSRV() { return s_MetaSRV; }
		static ID3D11Buffer* const GetDataBuffer() { return s_DataBuffer; }
		static ID3D11ShaderResourceView* const GetDataSRV() { return s_DataSRV; }
		static void Pad();
		static void Uninit();
		static void Update();
		static DynamicIndexDivision Create(unsigned int count, unsigned int* data = nullptr);

	private:
		unsigned int m_Id = UINT_MAX;
	public:
		void SetData(const unsigned int* data);
		const META& GetMetaData() const { return s_Meta[m_Id]; }
		unsigned int GetData(const unsigned int index) const { 
			if (index > s_Meta[m_Id].count) return UINT_MAX;
			return s_Data[s_Meta[m_Id].offset + index]; 
		}
		void SetData(const unsigned int index, const unsigned int value) {
			if (index > s_Meta[m_Id].count) return;
			s_Data[s_Meta[m_Id].offset + index] = value;
			s_NeedUpdateBuffer = true;
		}
		void SetData(const unsigned int* value, const unsigned int size) {
			memcpy(s_Data.data() + s_Meta[m_Id].offset, value, size);
			s_NeedUpdateBuffer = true;
		}

		void Release() {
			if (m_Id != UINT_MAX) {
				s_Meta[m_Id].count = 0;
				s_Meta[m_Id].padding = 0;
				s_EmptyIds.insert(m_Id);
				m_Id = UINT_MAX;
			}
		}

		operator bool() const {
			return m_Id != UINT_MAX;
		}

		operator unsigned int() const {
			return m_Id;
		}
	};

}

