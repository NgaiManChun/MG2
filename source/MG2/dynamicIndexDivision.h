// =======================================================
// dynamicIndexDivision.h
// 
// 汎用連続unsigned intバッファ
// 主に親ノーツIDやマテリアルIDなどに使う
// =======================================================
#pragma once
#include <vector>
#include <set>
#include "dataType.h"
#include "buffer.h"

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;

namespace MG {
	class DynamicIndexDivision {
	private:
		static inline std::vector<BOOKMARK> s_Bookmarks{};
		static inline std::vector<unsigned int> s_Data{};
		static inline std::set<unsigned int> s_EmptyIds{};
		static inline ID3D11Buffer* s_BookmarkBuffer = nullptr;
		static inline ID3D11ShaderResourceView* s_BookmarkSRV = nullptr;
		static inline ID3D11Buffer* s_DataBuffer = nullptr;
		static inline ID3D11ShaderResourceView* s_DataSRV = nullptr;
		static inline unsigned int s_BookmarkCapcity = 0;
		static inline unsigned int s_DataCapcity = 0;
		static inline bool s_NeedUpdateBuffer = false;

	public:
		static ID3D11Buffer* const GetBookmarkBuffer() { return s_BookmarkBuffer; }
		static ID3D11ShaderResourceView* const GetBookmarkSRV() { return s_BookmarkSRV; }
		static ID3D11Buffer* const GetDataBuffer() { return s_DataBuffer; }
		static ID3D11ShaderResourceView* const GetDataSRV() { return s_DataSRV; }
		
		static void Uninit();
		static void Update();
		static DynamicIndexDivision Create(unsigned int count, unsigned int* data = nullptr);
		static void Pad();

	private:
		unsigned int m_Id = UINT_MAX;

	public:
		BUFFER_HANDLE_OPERATOR(DynamicIndexDivision)

		const BOOKMARK& GetBookmarkData() const { return s_Bookmarks[m_Id]; }

		unsigned int GetData(const unsigned int index) const 
		{ 
			if (index > s_Bookmarks[m_Id].count) return UINT_MAX;
			return s_Data[s_Bookmarks[m_Id].offset + index]; 
		}

		void SetData(const unsigned int index, const unsigned int value) 
		{
			if (index > s_Bookmarks[m_Id].count) return;
			s_Data[s_Bookmarks[m_Id].offset + index] = value;
			s_NeedUpdateBuffer = true;
		}

		void SetData(const unsigned int* value, const unsigned int size) 
		{
			memcpy(s_Data.data() + s_Bookmarks[m_Id].offset, value, size);
			s_NeedUpdateBuffer = true;
		}

		void Release() 
		{
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

