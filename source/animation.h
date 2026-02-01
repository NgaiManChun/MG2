#pragma once
#include <vector>
#include <set>
#include <unordered_map>
#include "dataType.h"

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;

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
		static void Uninit();
		static Animation Create(const char* filename);
		
	private:
		unsigned int m_Id = UINT_MAX;
	public:

		const DATA& GetData() const { return s_Data[m_Id]; }

		unsigned int GetFrameCount() const { return s_Data[m_Id].frameCount; }

		unsigned int GetDuration() const { return s_Data[m_Id].duration; }

		void Release() {
			if (m_Id != UINT_MAX) {
				s_Data[m_Id] = {};

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

		bool operator ==(Animation& other) const {
			return m_Id == other.m_Id;
		}

		Animation& operator=(const unsigned int& id) { m_Id = id; }
		Animation() = default;
		Animation(const unsigned int& id) :m_Id(id) {}
	};

}

