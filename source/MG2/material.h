// =======================================================
// material.h
// 
// マテリアルのバッファ
// =======================================================
#pragma once
#include <vector>
#include <set>
#include "texture.h"
#include "buffer.h"

namespace MG {
	class Material {
	public:
		struct MATERIAL
		{
			Vector4 base = RGBA::WHITE;
			Vector4 emissive;
			Vector4 specular;
			float metallic;
			float roughness;
			float shininess;
			Texture baseTexture;
			Texture normalTexture;
			Texture opacityTexture;
			MATERIAL_TYPE type;
			unsigned int opaque;
		};

	private:
		static inline std::vector<MATERIAL> s_Data;
		static inline std::set<unsigned int> s_EmptyIds{};
		static inline ID3D11Buffer* s_Buffer = nullptr;
		static inline ID3D11ShaderResourceView* s_SRV = nullptr;
		static inline unsigned int s_Capcity = 0;
		static inline bool s_NeedUpdateBuffer = false;

	public:
		static ID3D11ShaderResourceView* const GetSRV() { return s_SRV; }

		static Material Create(const MATERIAL& data = {}) 
		{
			Material key = {};
			if (s_EmptyIds.empty()) {
				s_Data.push_back(data);
				key.m_Id = static_cast<unsigned int>(s_Data.size()) - 1;
			}
			else {
				key.m_Id = *s_EmptyIds.begin();
				s_EmptyIds.erase(s_EmptyIds.begin());
				s_Data[key.m_Id] = data;
			}

			s_NeedUpdateBuffer = true;

			return key;
		}

		static void Uninit();
		static void Update();

	private:
		unsigned int m_Id = UINT_MAX;

	public:
		BUFFER_HANDLE_OPERATOR(Material)

		void SetData(const MATERIAL& data) {
			s_Data[m_Id] = data;
			s_NeedUpdateBuffer = true;
		}

		const MATERIAL& GetData() const { return s_Data[m_Id]; }

		void Release() {
			if (m_Id != UINT_MAX) {
				s_EmptyIds.insert(m_Id);
				m_Id = UINT_MAX;
			}
		}

	};

}

