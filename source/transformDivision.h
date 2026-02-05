// =======================================================
// transformDivision.h
// 
// 連続トランスフォームバッファ、
// 主にモデルの階層ノーツに使う
// =======================================================
#pragma once
#include <vector>
#include <set>
#include "dataType.h"

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11UnorderedAccessView;
struct ID3D11ComputeShader;

namespace MG {
	class TransformDivision {
		struct BOOKMARK {
			unsigned int offset;
			unsigned int count;
			unsigned int padding;
		};
	private:
		static inline std::vector<BOOKMARK> s_Bookmarks{};
		static inline std::set<unsigned int> s_EmptyIds{};

		static inline ID3D11Buffer* s_BookmarkBuffer = nullptr;
		static inline ID3D11ShaderResourceView* s_BookmarkSRV = nullptr;
		static inline ID3D11Buffer* s_DataBuffer = nullptr;
		static inline ID3D11ShaderResourceView* s_DataSRV = nullptr;
		static inline ID3D11UnorderedAccessView* s_DataUAV = nullptr;
		static inline ID3D11ComputeShader* s_PadCS = nullptr;
		static inline unsigned int s_BookmarkCapcity = 0;
		static inline unsigned int s_DataCapcity = 0;
		static inline unsigned int s_DataSize = 0;
		static const constexpr unsigned int META_INTERVAL = 50;
		static const constexpr unsigned int DATA_INTERVAL = 5000;

	public:
		static ID3D11Buffer* const GetBookmarkBuffer() { return s_BookmarkBuffer; }
		static ID3D11ShaderResourceView* const GetBookmarkSRV() { return s_BookmarkSRV; }
		static ID3D11Buffer* const GetDataBuffer() { return s_DataBuffer; }
		static ID3D11ShaderResourceView* const GetDataSRV() { return s_DataSRV; }
		static ID3D11UnorderedAccessView* const GetDataUAV() { return s_DataUAV; }
		static bool ReserveMeta(unsigned int newCapcity);
		static bool ReserveData(unsigned int newCapcity);
		static void Pad();
		static void Init();
		static void Uninit();
		static TransformDivision Create(unsigned int count, const TRANSFORM* data = nullptr);

	private:
		unsigned int m_Id = UINT_MAX;
	public:
		void SetData(TRANSFORM* data);
		const BOOKMARK& GetBookmarkData() const { return s_Bookmarks[m_Id]; }

		void Release() {
			if (m_Id != UINT_MAX) {
				s_Bookmarks[m_Id].count = 0;
				s_Bookmarks[m_Id].padding = 0;
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