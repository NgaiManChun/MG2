#pragma once
#include <vector>
#include <set>
#include <unordered_map>
#include <string>
#include "dataType.h"

struct ID3D11ShaderResourceView;

namespace MG {
	class Texture {
		struct PAIR {
			ID3D11ShaderResourceView* srv;
			size_t width;
			size_t height;
			std::string name;
		};
	private:
		static inline std::vector<PAIR> s_SRVs;
		static inline std::unordered_map<std::string, unsigned int> s_NameMap;
		static inline std::set<unsigned int> s_EmptyIds{};
	public:
		static void Init();
		static void Uninit();
		static void Update();
		static unsigned int Count() {
			return static_cast<unsigned int>(s_SRVs.size());
		}

		static Texture Create(const char* name);

		static Texture Create(const char* name, const unsigned char* data, size_t size);

		static Texture Create(const char* name, ID3D11ShaderResourceView* srv, size_t width, size_t height);

	private:
		unsigned int m_Id = UINT_MAX;
	public:

		ID3D11ShaderResourceView* const GetSRV() const { return (m_Id != UINT_MAX) ? s_SRVs[m_Id].srv : nullptr; }

		void Release();

		operator bool() const {
			return m_Id != UINT_MAX;
		}

		operator unsigned int() const {
			return m_Id;
		}

		bool operator <(Texture& other) const {
			return m_Id < other.m_Id;
		}

		bool operator ==(Texture& other) const {
			return m_Id == other.m_Id;
		}

		Texture& operator=(const unsigned int& id) { m_Id = id; }
		Texture() = default;
		Texture(const unsigned int& id) :m_Id(id) {}
	};

}

