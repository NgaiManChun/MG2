// =======================================================
// matrixDivision.h
// 
// 連続マトリックスバッファ
// =======================================================
#pragma once
#include <vector>
#include <set>
#include "buffer.h"

namespace MG {

	struct Matrix4x4;

	class MatrixDivision {
	private:
		static inline std::vector<BOOKMARK> s_Bookmarks{};
		static inline std::set<unsigned int> s_EmptyIds{};
		static inline ID3D11Buffer* s_BookmarkBuffer = nullptr;
		static inline ID3D11ShaderResourceView* s_BookmarkSRV = nullptr;
		static inline ID3D11Buffer* s_DataBuffer = nullptr;
		static inline ID3D11ShaderResourceView* s_DataSRV = nullptr;
		static inline ID3D11UnorderedAccessView* s_DataUAV = nullptr;
		static inline unsigned int s_BookmarkCapcity = 0;
		static inline unsigned int s_DataCapcity = 0;
		static inline unsigned int s_DataCount = 0;

	public:
		static ID3D11Buffer* const GetBookmarkBuffer() { return s_BookmarkBuffer; }
		static ID3D11ShaderResourceView* const GetBookmarkSRV() { return s_BookmarkSRV; }
		static ID3D11Buffer* const GetDataBuffer() { return s_DataBuffer; }
		static ID3D11ShaderResourceView* const GetDataSRV() { return s_DataSRV; }
		static ID3D11UnorderedAccessView* const GetDataUAV() { return s_DataUAV; }
		
		static void Uninit();
		static MatrixDivision Create(unsigned int count, const Matrix4x4* data = nullptr);
		static void Pad();

	private:
		unsigned int m_Id = UINT_MAX;

	public:
		BUFFER_HANDLE_OPERATOR(MatrixDivision)

		const BOOKMARK& GetBookmarkData() const { return s_Bookmarks[m_Id]; }

		void Release() 
		{
			if (m_Id != UINT_MAX) {
				s_Bookmarks[m_Id].count = 0;
				s_Bookmarks[m_Id].padding = 0;
				s_EmptyIds.insert(m_Id);
				m_Id = UINT_MAX;
			}
		}

		void SetData(Matrix4x4* data);

		
	};

}

