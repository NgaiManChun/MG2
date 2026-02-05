// =======================================================
// boneDivision.h
// 
// 連続ボーンバッファ
// =======================================================
#pragma once
#include <vector>
#include <set>

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11ComputeShader;

namespace MG {

	struct BONE;

	class BoneDivision {
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
		static inline ID3D11ComputeShader* s_PadCS = nullptr;
		static inline unsigned int s_BookmarkCapcity = 0;
		static inline unsigned int s_DataCapcity = 0;
		static inline unsigned int s_DataSize = 0;
		static const constexpr unsigned int DATA_INTERVAL = 5000;
	public:
		static ID3D11Buffer* const GetBookmarkBuffer() { return s_BookmarkBuffer; }
		static ID3D11ShaderResourceView* const GetBookmarkSRV() { return s_BookmarkSRV; }
		static ID3D11Buffer* const GetDataBuffer() { return s_DataBuffer; }
		static ID3D11ShaderResourceView* const GetDataSRV() { return s_DataSRV; }

		static void Pad();
		static void Init();
		static void Uninit();
		static BoneDivision Create(unsigned int count, const BONE* data = nullptr);

	private:
		unsigned int m_Id = UINT_MAX;
	public:
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

		bool operator ==(BoneDivision& other) const {
			return m_Id == other.m_Id;
		}

		bool operator !=(BoneDivision& other) const {
			return m_Id == other.m_Id;
		}

		operator unsigned int() const {
			return m_Id;
		}
	};

}

