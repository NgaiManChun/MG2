#define _CRT_SECURE_NO_WARNINGS
#include "texture.h"
#include "renderer.h"
#include "MGUtility.h"

namespace MG {

	Texture Texture::_Create(const char* name, ID3D11ShaderResourceView* srv, size_t width, size_t height)
	{
		Texture key = {};
		PAIR pair{};
		pair.srv = srv;
		pair.width = width;
		pair.height = height;
		pair.name = name;

		if (s_EmptyIds.empty()) {
			s_SRVs.push_back(pair);
			key.m_Id = s_SRVs.size() - 1;
		}
		else {
			key.m_Id = *s_EmptyIds.begin();
			s_EmptyIds.erase(s_EmptyIds.begin());
			s_SRVs[key.m_Id] = pair;
		}

		s_NameMap[name] = key.m_Id;

		return key;
	}

	void Texture::Uninit()
	{
		for (PAIR& pair : s_SRVs) {
			SAFE_RELEASE(pair.srv);
		}
		s_SRVs.clear();
		s_NameMap.clear();
		s_EmptyIds.clear();
	}

	Texture Texture::GetTexture(const char* filename)
	{
		Texture key = {};
		if (s_NameMap.count(filename) > 0) {
			key.m_Id = s_NameMap.at(filename);
			return key;
		}
		return key;
	}

	Texture Texture::Create(const char* filename) {
		
		Texture key = GetTexture(filename);
		if (key) return key;

		// ƒƒCƒh•¶š‚É•ÏŠ·
		wchar_t wFileName[512];
		mbstowcs(wFileName, filename, strlen(filename) + 1);

		// ‰æ‘œ“Ç‚İ‚İ
		TexMetadata metadata;
		ScratchImage image;
		LoadFromWICFile(wFileName, WIC_FLAGS_NONE, &metadata, image);

		// SRVì¬
		ID3D11ShaderResourceView* srv;
		CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &srv);

		return _Create(filename, srv, metadata.width, metadata.height);
	}

	Texture Texture::Create(const char* name, const unsigned char* data, size_t size) {

		Texture key = GetTexture(name);
		if (key) return key;

		// ‰æ‘œ“Ç‚İ‚İ
		TexMetadata metadata;
		ScratchImage image;
		LoadFromWICMemory(data, size, WIC_FLAGS_NONE, &metadata, image);

		// SRVì¬
		ID3D11ShaderResourceView* srv;
		CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &srv);

		return _Create(name, srv, metadata.width, metadata.height);
	}

	Texture Texture::Create(const char* name, ID3D11ShaderResourceView* srv, size_t width, size_t height)
	{
		Texture key = GetTexture(name);
		if (key) return key;

		return _Create(name, srv, width, height);
	}

	void Texture::Release() 
	{
		if (m_Id != UINT_MAX) {
			PAIR& pair = s_SRVs[m_Id];
			s_NameMap.erase(pair.name);
			pair.name.clear();
			SAFE_RELEASE(pair.srv);
			s_EmptyIds.insert(m_Id);
			m_Id = UINT_MAX;
		}
	}
} // namespace MG
