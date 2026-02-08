// =======================================================
// vertexDivision.h
// 
// 連続頂点インデックスバッファ
// =======================================================
#pragma once
#include <vector>
#include <set>
#include "dataType.h"
#include "buffer.h"

namespace MG {
	class VertexIndexDivision {
	private:
		static inline std::vector<BOOKMARK> s_Bookmarks{};
		static inline std::set<unsigned int> s_EmptyIds{};
		static inline ID3D11Buffer* s_DataBuffer = nullptr;
		static inline unsigned int s_DataCapcity = 0;
		static inline unsigned int s_DataCount = 0;

	public:
		static ID3D11Buffer* const GetDataBuffer() { return s_DataBuffer; }
		
		static void Uninit();
		static VertexIndexDivision Create(unsigned int count, const unsigned int* data = nullptr);
		static void Pad();

	private:
		unsigned int m_Id = UINT_MAX;

	public:
		BUFFER_HANDLE_OPERATOR(VertexIndexDivision)

		const BOOKMARK& GetBookmarkData() const { return s_Bookmarks[m_Id]; }

		void Release() {
			if (m_Id != UINT_MAX) {
				s_Bookmarks[m_Id].count = 0;
				s_Bookmarks[m_Id].padding = 0;
				s_EmptyIds.insert(m_Id);
				m_Id = UINT_MAX;
			}
		}

		void SetData(const unsigned int* data);
	};

}