#pragma once
#include <vector>
#include <set>
#include "matrix4x4.h"

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
struct ID3D11Texture2D;
struct ID3D11RenderTargetView;

namespace MG {

	class AtlasTexture {
	private:
		static inline std::set<unsigned int> s_EmptyIds{};

		static inline ID3D11Texture2D* s_TextureArray = nullptr;
		static inline ID3D11ShaderResourceView* s_SRV = nullptr;
		static inline std::vector<ID3D11RenderTargetView*> s_RTVs;
		static inline unsigned int s_Capcity = 0;
		static inline unsigned int s_Size = 0;
		static const constexpr unsigned int WIDTH = 1024;
		static const constexpr unsigned int HEIGHT = 1024;
		static const constexpr unsigned int DATA_INTERVAL = 128;

	public:
		static void Init();
		static void Uninit();
		static void Update();

		static ID3D11ShaderResourceView* const GetTextureArraySRV() { return s_SRV; }

		static void LoadTexture(const char* filename, unsigned int index);

		static AtlasTexture Create(const char* filename) {
			AtlasTexture key = {};
			if (s_EmptyIds.empty()) {
				key.m_Id = s_Size;
				s_Size++;
			}
			else {
				auto it = std::next(s_EmptyIds.begin(), 1);
				key.m_Id = *it;
				s_EmptyIds.erase(it);
			}

			LoadTexture(filename, key.m_Id);

			return key;
		}
	private:
		unsigned int m_Id = UINT_MAX;
	public:
		void Release() {
			if (m_Id != UINT_MAX) {
				s_EmptyIds.insert(m_Id);
				m_Id = UINT_MAX;
			}
		}

		operator bool() {
			return m_Id != UINT_MAX;
		}
	};

}

