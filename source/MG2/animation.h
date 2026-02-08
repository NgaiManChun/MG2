// =======================================================
// animation.h
// 
// アニメーションクラス
// =======================================================
#pragma once
#include <vector>
#include <set>
#include <unordered_map>
#include "buffer.h"

namespace MG {
	class Animation {
		struct DATA {
			unsigned int frameCount;
			unsigned int duration;
			std::unordered_map<std::string, std::vector<TRANSFORM>> channels;
		};

	private:
		static inline std::vector<DATA> s_Data;
		static inline std::unordered_map<std::string, unsigned int> s_NameMap;
		static inline std::set<unsigned int> s_EmptyIds{};

	public:
		static Animation Create(const char* filename);

		static void Uninit()
		{
			s_Data.clear();
			s_EmptyIds.clear();
			s_NameMap.clear();
		}

	private:
		unsigned int m_Id = UINT_MAX;

	public:
		BUFFER_HANDLE_OPERATOR(Animation)

		const DATA& GetData() const { return s_Data[m_Id]; }
		unsigned int GetFrameCount() const { return s_Data[m_Id].frameCount; }
		unsigned int GetDuration() const { return s_Data[m_Id].duration; }

		void Release() 
		{
			if (m_Id != UINT_MAX) {
				s_Data[m_Id] = {};

				s_EmptyIds.insert(m_Id);
				m_Id = UINT_MAX;
			}
		}
	};

}

