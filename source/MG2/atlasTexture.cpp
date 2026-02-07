#define _CRT_SECURE_NO_WARNINGS
#include "atlasTexture.h"
#include "renderer.h"
#include "MGUtility.h"

namespace MG {

	void AtlasTexture::Init()
	{

		unsigned int newCapcity = max(ceil(static_cast<float>(s_Size) / DATA_INTERVAL), 1) * DATA_INTERVAL;

		ID3D11Device* device = Renderer::GetDevice();

		ID3D11Texture2D* textureArray;

		D3D11_TEXTURE2D_DESC texture2DDesc{};
		texture2DDesc.Width = WIDTH;
		texture2DDesc.Height = HEIGHT;
		texture2DDesc.MipLevels = 0;
		texture2DDesc.ArraySize = newCapcity;
		texture2DDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texture2DDesc.SampleDesc.Count = 1;
		texture2DDesc.SampleDesc.Quality = 0;
		texture2DDesc.Usage = D3D11_USAGE_DEFAULT;
		texture2DDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		texture2DDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		device->CreateTexture2D(&texture2DDesc, NULL, &textureArray);

		if (textureArray && s_TextureArray) {
			Renderer::GetDeviceContext()->CopyResource(textureArray, s_TextureArray);
			for (ID3D11RenderTargetView* rtv : s_RTVs) {
				SAFE_RELEASE(rtv)
			}
			s_RTVs.clear();
			SAFE_RELEASE(s_SRV)
		}

		s_TextureArray = textureArray;

		if (s_SRV == nullptr) {
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Texture2DArray.MipLevels = -1;
			srvDesc.Texture2DArray.MostDetailedMip = 0;
			srvDesc.Texture2DArray.FirstArraySlice = 0;
			srvDesc.Texture2DArray.ArraySize = newCapcity;

			Renderer::GetDevice()->CreateShaderResourceView(
				s_TextureArray,
				&srvDesc,
				&s_SRV
			);
		}

		if (s_TextureArray) {
			s_RTVs.clear();

			for (unsigned int i = 0; i < newCapcity; i++) {
				ID3D11RenderTargetView* rtv;
				D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
				rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
				rtvDesc.Texture2DArray.MipSlice = 0;
				rtvDesc.Texture2DArray.FirstArraySlice = i;
				rtvDesc.Texture2DArray.ArraySize = 1;
				device->CreateRenderTargetView(s_TextureArray, NULL, &rtv);
				s_RTVs.push_back(rtv);
			}

			s_Capcity = newCapcity;
		}
	}

	void AtlasTexture::Uninit()
	{
		for (ID3D11RenderTargetView* rtv : s_RTVs) {
			SAFE_RELEASE(rtv)
		}
		s_RTVs.clear();
		SAFE_RELEASE(s_SRV)
		SAFE_RELEASE(s_TextureArray)
	}

	void AtlasTexture::LoadTexture(const char* filename, unsigned int index)
	{
		if (s_Size > s_Capcity) {
			Init();
		}

		wchar_t wFileName[512];
		mbstowcs(wFileName, filename, strlen(filename) + 1);

		TexMetadata metadata;
		ScratchImage image;
		ID3D11ShaderResourceView* srv;

		LoadFromWICFile(wFileName, WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), metadata, &srv);

		auto shaderSet = Renderer::GetShaderSet(SHADER_TYPE_TEXTURE_COPY);
		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();

		deviceContext->OMSetRenderTargets(1, &s_RTVs[index], nullptr);
		Renderer::SetViewport(WIDTH, HEIGHT);
		deviceContext->VSSetShader(shaderSet.vertexShader, NULL, 0);
		deviceContext->IASetInputLayout(shaderSet.inputLayout);
		deviceContext->PSSetShader(shaderSet.pixelShader, NULL, 0);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		deviceContext->PSSetShaderResources(SLOT_TEXTURE, 1, &srv);

		deviceContext->Draw(4, 0);

		SAFE_RELEASE(srv);
		image.Release();

		deviceContext->GenerateMips(s_SRV);
	}

	void AtlasTexture::Update()
	{
	}
} // namespace MG
