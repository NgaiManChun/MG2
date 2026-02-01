#define _CRT_SECURE_NO_WARNINGS
#include "texture.h"
#include "renderer.h"
#include "MGUtility.h"

namespace MG {

	void Texture::Init()
	{
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

	void Texture::Update()
	{
	}

	Texture Texture::Create(const char* name) {
		Texture key = {};
		if (s_NameMap.count(name) > 0) {
			key.m_Id = s_NameMap.at(name);
			return key;
		}

		// Load File
		wchar_t wFileName[512];
		mbstowcs(wFileName, name, strlen(name) + 1);

		ID3D11ShaderResourceView* srv;
		TexMetadata metadata;
		ScratchImage image;

		LoadFromWICFile(wFileName, WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &srv);

		PAIR pair{};
		pair.srv = srv;
		pair.width = metadata.width;
		pair.height = metadata.height;
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

	Texture Texture::Create(const char* name, const unsigned char* data, size_t size) {
		Texture key = {};
		if (s_NameMap.count(name) > 0) {
			key.m_Id = s_NameMap.at(name);
			return key;
		}

		// Load File
		ID3D11ShaderResourceView* srv;
		TexMetadata metadata;
		ScratchImage image;

		LoadFromWICMemory(data, size, WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &srv);

		PAIR pair{};
		pair.srv = srv;
		pair.width = metadata.width;
		pair.height = metadata.height;
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

	Texture Texture::Create(const char* name, ID3D11ShaderResourceView* srv, size_t width, size_t height)
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

	void Texture::Release() {
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
