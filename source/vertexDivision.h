#pragma once
#include <vector>
#include <set>
#include "dataType.h"

struct ID3D11Buffer;
struct ID3D11ComputeShader;

namespace MG {
	class VertexDivision {
		struct META {
			unsigned int offset;
			unsigned int count;
			unsigned int padding;
		};
	private:
		static inline std::vector<META> s_Meta{};
		static inline std::set<unsigned int> s_EmptyIds{};

		static inline ID3D11Buffer* s_DataBuffer = nullptr;
		static inline ID3D11ComputeShader* s_PadCS = nullptr;
		static inline unsigned int s_DataCapcity = 0;
		static inline unsigned int s_DataCount = 0;
		static const constexpr unsigned int DATA_INTERVAL = 5000;

	public:
		static ID3D11Buffer* const GetDataBuffer() { return s_DataBuffer; }
		static bool ReserveData(unsigned int newCapcity);
		static void Pad();
		static void Init();
		static void Uninit();
		static VertexDivision Create(unsigned int count, const VERTEX* data = nullptr);

	private:
		unsigned int m_Id = UINT_MAX;
	public:
		void SetData(VERTEX* data);
		const META& GetMetaData() const { return s_Meta[m_Id]; }

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