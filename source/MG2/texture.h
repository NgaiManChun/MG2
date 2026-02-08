// =======================================================
// texture.h
// 
// テクスチャクラス
// =======================================================
#pragma once
#include <vector>
#include <set>
#include <unordered_map>
#include <string>
#include "buffer.h"

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

		// Createの共通部分
		static Texture _Create(const char* name, ID3D11ShaderResourceView* srv, size_t width, size_t height);

	public:
		static unsigned int Count() { return static_cast<unsigned int>(s_SRVs.size()); }

		static void Uninit();

		static Texture GetTexture(const char* filename);

		// ファイルパスから作成
		static Texture Create(const char* filename);

		// バイナリデータから作成
		static Texture Create(const char* name, const unsigned char* data, size_t size);

		// SRVから作成
		static Texture Create(const char* name, ID3D11ShaderResourceView* srv, size_t width, size_t height);

	private:
		unsigned int m_Id = UINT_MAX;

	public:
		BUFFER_HANDLE_OPERATOR(Texture)

		ID3D11ShaderResourceView* const GetSRV() const { return (m_Id != UINT_MAX) ? s_SRVs[m_Id].srv : nullptr; }
		bool operator <(Texture& other) const { return m_Id < other.m_Id; }

		void Release();

	};

}

