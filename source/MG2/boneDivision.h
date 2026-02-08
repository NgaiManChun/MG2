// =======================================================
// boneDivision.h
// 
// 連続ボーンバッファ
// メッシュごとのボーン配列（offsetMatrixとnodeId）
// =======================================================
#pragma once
#include <vector>
#include <set>
#include "dataType.h"
#include "buffer.h"

namespace MG {

	class BoneDivision {
	private:
		static inline std::vector<BOOKMARK> s_Bookmarks{};
		static inline std::set<unsigned int> s_EmptyIds{};
		static inline ID3D11Buffer* s_BookmarkBuffer = nullptr;
		static inline ID3D11ShaderResourceView* s_BookmarkSRV = nullptr;
		static inline ID3D11Buffer* s_DataBuffer = nullptr;
		static inline ID3D11ShaderResourceView* s_DataSRV = nullptr;
		static inline unsigned int s_BookmarkCapcity = 0;
		static inline unsigned int s_DataCapcity = 0;
		static inline unsigned int s_DataCount = 0;

	public:
		static ID3D11Buffer* const GetBookmarkBuffer() { return s_BookmarkBuffer; }
		static ID3D11ShaderResourceView* const GetBookmarkSRV() { return s_BookmarkSRV; }
		static ID3D11Buffer* const GetDataBuffer() { return s_DataBuffer; }
		static ID3D11ShaderResourceView* const GetDataSRV() { return s_DataSRV; }
		
		static void Uninit();
		static BoneDivision Create(unsigned int count, const BONE* data = nullptr);
		static void Pad();

	private:
		unsigned int m_Id = UINT_MAX;

	public:
		BUFFER_HANDLE_OPERATOR(BoneDivision)

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

		
	};

}

