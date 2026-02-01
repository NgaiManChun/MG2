#pragma once
#include <vector>
#include <set>
#include "matrix4x4.h"

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;
struct ID3D11ComputeShader;

namespace MG {
	class MatrixDivision {
		struct META {
			unsigned int offset;
			unsigned int count;
			unsigned int padding;
		};
	private:
		static inline std::vector<META> s_Meta{};
		static inline std::set<unsigned int> s_EmptyIds{};

		static inline ID3D11Buffer* s_MetaBuffer = nullptr;
		static inline ID3D11ShaderResourceView* s_MetaSRV = nullptr;
		static inline ID3D11Buffer* s_DataBuffer = nullptr;
		static inline ID3D11ShaderResourceView* s_DataSRV = nullptr;
		static inline ID3D11UnorderedAccessView* s_DataUAV = nullptr;
		static inline ID3D11ComputeShader* s_PadCS = nullptr;
		static inline unsigned int s_MetaCapcity = 0;
		static inline unsigned int s_DataCapcity = 0;
		static inline unsigned int s_DataSize = 0;
		static const constexpr unsigned int META_INTERVAL = 50;
		static const constexpr unsigned int DATA_INTERVAL = 5000;
	public:
		static ID3D11Buffer* const GetMetaBuffer() { return s_MetaBuffer; }
		static ID3D11ShaderResourceView* const GetMetaSRV() { return s_MetaSRV; }
		static ID3D11Buffer* const GetDataBuffer() { return s_DataBuffer; }
		static ID3D11ShaderResourceView* const GetDataSRV() { return s_DataSRV; }
		static ID3D11UnorderedAccessView* const GetDataUAV() { return s_DataUAV; }
		static bool ReserveMeta(unsigned int newCapcity);
		static bool ReserveData(unsigned int newCapcity);
		static void Pad();
		static void Init();
		static void Uninit();
		static MatrixDivision Create(unsigned int count, const Matrix4x4* data = nullptr);

	private:
		unsigned int m_Id = UINT_MAX;
	public:
		void SetData(Matrix4x4* data);
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

		bool operator ==(MatrixDivision& other) const {
			return m_Id == other.m_Id;
		}

		bool operator !=(MatrixDivision& other) const {
			return m_Id == other.m_Id;
		}

		operator unsigned int() const {
			return m_Id;
		}
	};

}

