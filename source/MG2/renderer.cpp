#include "renderer.h"
#include <io.h>
#include "MGUtility.h"
#include "MGResource.h"
#include "csvResource.h"
#include <unordered_map>

#include <dxgi1_6.h>
#include <d3d11.h>
#include <wrl.h>
#include <iostream>
#include <fstream>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

using Microsoft::WRL::ComPtr;

#define STARTS_WITH(STR, PREFIX) \
(STR.compare(0, std::string(PREFIX).size(), std::string(PREFIX)) == 0)

void ReadFile(const char* filename, std::vector<char>& buffer)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file) {
		// サイズ取得
		size_t size = static_cast<size_t>(file.tellg());
		file.seekg(0, std::ios::beg);
		file.clear();

		buffer.resize(size);

		file.read(buffer.data(), size);
		file.close();
	}
}

void ParseInputLayout(MG::CSVResource& csv, std::vector<D3D11_INPUT_ELEMENT_DESC>& desc)
{
	for (auto row : csv.WithoutHeader()) {
		D3D11_INPUT_ELEMENT_DESC element{};
		element.SemanticName = row["SemanticName"];
		element.SemanticIndex = row["SemanticIndex"];
		element.Format = MG::Renderer::GetDXGIFormatByName(row["Format"]);
		element.InputSlot = row["InputSlot"];
		element.AlignedByteOffset = row["AlignedByteOffset"];
		element.InputSlotClass = MG::Renderer::GetInputClassByName(row["InputSlotClass"]);
		element.InstanceDataStepRate = row["InstanceDataStepRate"];
		desc.push_back(element);
	}
}

namespace MG {
	void Renderer::Init(HWND hWnd)
	{
		HRESULT hr = S_OK;

		ComPtr<IDXGIFactory6> factory;
		if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
			return;
		}

		ComPtr<IDXGIAdapter1> adapter;
		// DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE: 高性能GPUを優先
		if (FAILED(factory->EnumAdapterByGpuPreference(
			0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
			IID_PPV_ARGS(&adapter)
		))) {
			return;
		}

		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);


		// デバイス、スワップチェーン作成
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferCount = 2;
		swapChainDesc.BufferDesc.Width = MGUtility::GetScreenWidth();
		swapChainDesc.BufferDesc.Height = MGUtility::GetScreenHeight();
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.OutputWindow = hWnd; //GetWindow();
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		
		hr = D3D11CreateDeviceAndSwapChain(
			adapter.Get(), // NULL,
			D3D_DRIVER_TYPE_UNKNOWN, // D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			0,
			NULL,
			0,
			D3D11_SDK_VERSION,
			&swapChainDesc,
			&s_SwapChain,
			&s_Device,
			&s_FeatureLevel,
			&s_DeviceContext);

		// レンダーターゲットビュー作成
		ID3D11Texture2D* renderTarget{};
		s_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&renderTarget);
		s_Device->CreateRenderTargetView(renderTarget, NULL, &s_RenderTargetView);
		renderTarget->Release();


		// デプスステンシルバッファ作成
		ID3D11Texture2D* depthStencile{};
		D3D11_TEXTURE2D_DESC textureDesc{};
		textureDesc.Width = swapChainDesc.BufferDesc.Width;
		textureDesc.Height = swapChainDesc.BufferDesc.Height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_D16_UNORM;
		textureDesc.SampleDesc = swapChainDesc.SampleDesc;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = 0;
		s_Device->CreateTexture2D(&textureDesc, NULL, &depthStencile);

		// デプスステンシルビュー作成
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = textureDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Flags = 0;
		s_Device->CreateDepthStencilView(depthStencile, &depthStencilViewDesc, &s_DepthStencilView);
		depthStencile->Release();


		s_DeviceContext->OMSetRenderTargets(1, &s_RenderTargetView, s_DepthStencilView);


		// ビューポート設定
		D3D11_VIEWPORT viewport;
		viewport.Width = (FLOAT)MGUtility::GetScreenWidth();
		viewport.Height = (FLOAT)MGUtility::GetScreenHeight();
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		s_DeviceContext->RSSetViewports(1, &viewport);


		// ラスタライザステート設定
		{
			D3D11_RASTERIZER_DESC rasterizerDesc{};
			rasterizerDesc.DepthClipEnable = TRUE;
			rasterizerDesc.MultisampleEnable = FALSE;

			rasterizerDesc.FillMode = D3D11_FILL_SOLID;
			rasterizerDesc.CullMode = D3D11_CULL_BACK;
			s_Device->CreateRasterizerState(&rasterizerDesc, &s_RasterizerStates[RASTERIZER_STATE_SOLID_CULL_BACK]);

			rasterizerDesc.CullMode = D3D11_CULL_FRONT;
			s_Device->CreateRasterizerState(&rasterizerDesc, &s_RasterizerStates[RASTERIZER_STATE_SOLID_CULL_FRONT]);

			rasterizerDesc.CullMode = D3D11_CULL_NONE;
			s_Device->CreateRasterizerState(&rasterizerDesc, &s_RasterizerStates[RASTERIZER_STATE_SOLID_CULL_NONE]);

			rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
			s_Device->CreateRasterizerState(&rasterizerDesc, &s_RasterizerStates[RASTERIZER_STATE_WIREFRAME_CULL_NONE]);

			s_DeviceContext->RSSetState(s_RasterizerStates[RASTERIZER_STATE_SOLID_CULL_BACK]);
		}

		// ブレンドステート設定
		{
			D3D11_BLEND_DESC blendDesc{};
			blendDesc.AlphaToCoverageEnable = FALSE;
			blendDesc.IndependentBlendEnable = FALSE;
			blendDesc.RenderTarget[0].BlendEnable = TRUE;
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
			blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

			// 無効
			blendDesc.RenderTarget[0].BlendEnable = FALSE;
			s_Device->CreateBlendState(&blendDesc, &s_BlendStates[BLEND_STATE_NONE]);

			// αブレンド + AlphaToCoverage
			blendDesc.RenderTarget[0].BlendEnable = TRUE;
			blendDesc.AlphaToCoverageEnable = TRUE;
			s_Device->CreateBlendState(&blendDesc, &s_BlendStates[BLEND_STATE_ALPHA_A2C]);

			// αブレンド
			blendDesc.RenderTarget[0].BlendEnable = TRUE;
			blendDesc.AlphaToCoverageEnable = FALSE;
			s_Device->CreateBlendState(&blendDesc, &s_BlendStates[BLEND_STATE_ALPHA]);

			// 加算合成
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			s_Device->CreateBlendState(&blendDesc, &s_BlendStates[BLEND_STATE_ADD]);

			// 減算合成
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
			s_Device->CreateBlendState(&blendDesc, &s_BlendStates[BLEND_STATE_SUB]);

			// くりぬき
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
			blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
			blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
			s_Device->CreateBlendState(&blendDesc, &s_BlendStates[BLEND_STATE_DEST_OUT]);

			SetBlendState(BLEND_STATE_ALPHA);
		}
		

		// 深度ステンシルステート設定
		{
			D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
			ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
			depthStencilDesc.DepthEnable = TRUE;
			depthStencilDesc.StencilEnable = FALSE;

			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
			s_Device->CreateDepthStencilState(&depthStencilDesc, &s_DepthStates[DEPTH_STATE_COMPARISON_LESS]);

			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
			s_Device->CreateDepthStencilState(&depthStencilDesc, &s_DepthStates[DEPTH_STATE_COMPARISON_ALWAYS]);

			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
			s_Device->CreateDepthStencilState(&depthStencilDesc, &s_DepthStates[DEPTH_STATE_NO_WRITE_COMPARISON_LESS]);

			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
			s_Device->CreateDepthStencilState(&depthStencilDesc, &s_DepthStates[DEPTH_STATE_NO_WRITE_COMPARISON_ALWAYS]);

			s_DeviceContext->OMSetDepthStencilState(s_DepthStates[DEPTH_STATE_COMPARISON_LESS], NULL);
		}

		// サンプラーステート設定
		{
			D3D11_SAMPLER_DESC samplerDesc{};
			//samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.MipLODBias = 0.0f;
			samplerDesc.MaxAnisotropy = 4;
			samplerDesc.MinLOD = 0;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;


			ID3D11SamplerState* samplerState{};
			s_Device->CreateSamplerState(&samplerDesc, &samplerState);
			s_DeviceContext->PSSetSamplers(0, 1, &samplerState);
		}

		// 定数バッファ生成
		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = sizeof(float);

		// カメラ
		bufferDesc.ByteWidth = sizeof(CAMERA_CONSTANT);
		s_Device->CreateBuffer(&bufferDesc, NULL, &s_CameraConstantBuffer);
		s_DeviceContext->VSSetConstantBuffers(0, 1, &s_CameraConstantBuffer);
		s_DeviceContext->PSSetConstantBuffers(0, 1, &s_CameraConstantBuffer);
		s_DeviceContext->CSSetConstantBuffers(0, 1, &s_CameraConstantBuffer);
		s_DeviceContext->GSSetConstantBuffers(0, 1, &s_CameraConstantBuffer);

		// ライト
		bufferDesc.ByteWidth = sizeof(LIGHT_CONSTANT);
		s_Device->CreateBuffer(&bufferDesc, NULL, &s_LightConstantBuffer);
		s_DeviceContext->VSSetConstantBuffers(1, 1, &s_LightConstantBuffer);
		s_DeviceContext->PSSetConstantBuffers(1, 1, &s_LightConstantBuffer);
		s_DeviceContext->CSSetConstantBuffers(1, 1, &s_LightConstantBuffer);

		// モデル
		bufferDesc.ByteWidth = sizeof(MODEL_CONSTANT);
		s_Device->CreateBuffer(&bufferDesc, NULL, &s_ModelConstantBuffer);
		s_DeviceContext->VSSetConstantBuffers(2, 1, &s_ModelConstantBuffer);
		s_DeviceContext->PSSetConstantBuffers(2, 1, &s_ModelConstantBuffer);
		s_DeviceContext->CSSetConstantBuffers(2, 1, &s_ModelConstantBuffer);

		// メッシュ
		bufferDesc.ByteWidth = sizeof(MESH_CONSTANT);
		s_Device->CreateBuffer(&bufferDesc, NULL, &s_MeshConstantBuffer);
		s_DeviceContext->VSSetConstantBuffers(3, 1, &s_MeshConstantBuffer);
		s_DeviceContext->PSSetConstantBuffers(3, 1, &s_MeshConstantBuffer);
		s_DeviceContext->CSSetConstantBuffers(3, 1, &s_MeshConstantBuffer);
		
		// ComputeShader
		bufferDesc.ByteWidth = sizeof(CS_CONSTANT);
		s_Device->CreateBuffer(&bufferDesc, NULL, &s_CSConstantBuffer);
		s_DeviceContext->VSSetConstantBuffers(4, 1, &s_CSConstantBuffer);
		s_DeviceContext->CSSetConstantBuffers(4, 1, &s_CSConstantBuffer);

		// シングルモデル（Sky）
		bufferDesc.ByteWidth = sizeof(SINGLE_CONSTANT);
		s_Device->CreateBuffer(&bufferDesc, NULL, &s_SingleConstantBuffer);
		s_DeviceContext->VSSetConstantBuffers(5, 1, &s_SingleConstantBuffer);

		// パーティクル
		bufferDesc.ByteWidth = sizeof(PARTICLE_CONSTANT);
		s_Device->CreateBuffer(&bufferDesc, NULL, &s_ParticleConstantBuffer);
		s_DeviceContext->VSSetConstantBuffers(6, 1, &s_ParticleConstantBuffer);
		s_DeviceContext->PSSetConstantBuffers(6, 1, &s_ParticleConstantBuffer);
		s_DeviceContext->CSSetConstantBuffers(6, 1, &s_ParticleConstantBuffer);

		// 時間
		bufferDesc.ByteWidth = sizeof(TIME_CONSTANT);
		s_Device->CreateBuffer(&bufferDesc, NULL, &s_TimeConstantBuffer);
		s_DeviceContext->VSSetConstantBuffers(7, 1, &s_TimeConstantBuffer);
		s_DeviceContext->PSSetConstantBuffers(7, 1, &s_TimeConstantBuffer);
		s_DeviceContext->CSSetConstantBuffers(7, 1, &s_TimeConstantBuffer);
		

		// シェーダ読み込み
		{
			MGResource shaderResource("shader.pak");
			MGResource configResource("config.pak");
			MGResource inputLayoutResource("inputLayout.pak");

			auto& allFiles = shaderResource.GetAllFiles();
			for (auto pair : allFiles) {
				std::string name = pair.first;
				unsigned char* data = pair.second.data;
				size_t size = pair.second.size;

				if (STARTS_WITH(name, "CS/")) {
					ID3D11ComputeShader* shader = nullptr;
					s_Device->CreateComputeShader(data, size, NULL, &shader);
					s_ComputeShaders[name] = shader;
				}
				else if (STARTS_WITH(name, "GS/")) {
					ID3D11GeometryShader* shader = nullptr;
					s_Device->CreateGeometryShader(data, size, NULL, &shader);
					s_GeometryShaders[name] = shader;
				}
				else if (STARTS_WITH(name, "VS/")) {
					ID3D11VertexShader* shader = nullptr;
					s_Device->CreateVertexShader(data, size, NULL, &shader);
					s_VertexShaderSets[name] = { shader };
				}
				else if (STARTS_WITH(name, "PS/")) {
					ID3D11PixelShader* shader = nullptr;
					s_Device->CreatePixelShader(data, size, NULL, &shader);
					s_PixelShaders[name] = shader;
				}
			}
			
			// inputLayout読み込み
			auto fileView = configResource.GetFile("inputLayout.csv");
			if (fileView) {
				CSVResource listCsv(fileView.data, fileView.size);
				for (auto row : listCsv.WithoutHeader()) {
					std::string shaderName = row["shader"];
					std::string inputLayoutName = row["input_layout"];

					if (s_VertexShaderSets.count(shaderName) == 0)
						continue;

					auto inputLayoutFile = inputLayoutResource.GetFile(inputLayoutName.data());
					if (!inputLayoutFile)
						continue;
					
					// csvからD3D11_INPUT_ELEMENT_DESC
					CSVResource inputLayoutData(inputLayoutFile.data, inputLayoutFile.size);
					std::vector<D3D11_INPUT_ELEMENT_DESC> layoutElements;
					ParseInputLayout(inputLayoutData, layoutElements);

					auto shaderFile = shaderResource.GetFile(shaderName.data());
					ID3D11InputLayout* inputLayout = nullptr;
					s_Device->CreateInputLayout(layoutElements.data(),
						static_cast<UINT>(layoutElements.size()),
						shaderFile.data,
						shaderFile.size,
						&inputLayout);

					s_VertexShaderSets[shaderName].inputLayouts[inputLayoutName] = inputLayout;
					
				}
			}

			configResource.Release();
			inputLayoutResource.Release();
			shaderResource.Release();
		}


	}

	void Renderer::Uninit()
	{

		// シェーダ
		for (auto& pair : s_VertexShaderSets) {
			VERTEX_SHADER_SET& shaderSet = pair.second;
			for (auto& inputLayoutPair : shaderSet.inputLayouts) {
				SAFE_RELEASE(inputLayoutPair.second);
			}
			SAFE_RELEASE(shaderSet.vertexShader);
		}
		s_VertexShaderSets.clear();

		for (auto& pair : s_PixelShaders) {
			SAFE_RELEASE(pair.second);
		}
		s_PixelShaders.clear();

		for (auto& pair : s_ComputeShaders) {
			SAFE_RELEASE(pair.second);
		}
		s_ComputeShaders.clear();

		for (auto& pair : s_GeometryShaders) {
			SAFE_RELEASE(pair.second);
		}
		s_GeometryShaders.clear();
		
		// 定数バッファ
		SAFE_RELEASE(s_CameraConstantBuffer);
		SAFE_RELEASE(s_LightConstantBuffer);
		SAFE_RELEASE(s_ModelConstantBuffer);
		SAFE_RELEASE(s_MeshConstantBuffer);
		SAFE_RELEASE(s_SingleConstantBuffer);
		SAFE_RELEASE(s_ParticleConstantBuffer);
		SAFE_RELEASE(s_TimeConstantBuffer);
		
		// 深度ステート
		for (int i = 0; i < DEPTH_STATE_MAX; i++) {
			SAFE_RELEASE(s_DepthStates[i])
		}

		// ブレンドステート
		for (int i = 0; i < BLEND_STATE_MAX; i++) {
			SAFE_RELEASE(s_BlendStates[i])
		}

		// ラスタライザステート
		for (int i = 0; i < RASTERIZER_STATE_MAX; i++) {
			SAFE_RELEASE(s_RasterizerStates[i])
		}

		if (s_DeviceContext != nullptr) {
			s_DeviceContext->ClearState();
		}

		SAFE_RELEASE(s_RenderTargetView)
		SAFE_RELEASE(s_SwapChain)
		SAFE_RELEASE(s_DeviceContext)
		SAFE_RELEASE(s_Device)
	}


	void Renderer::Begin()
	{
		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		s_DeviceContext->ClearRenderTargetView(s_RenderTargetView, clearColor);
		s_DeviceContext->ClearDepthStencilView(s_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}



	void Renderer::End()
	{
		s_SwapChain->Present(0, 0);
	}

	void Renderer::SetDepthState(DEPTH_STATE state)
	{
		s_DeviceContext->OMSetDepthStencilState(s_DepthStates[state], NULL);
	}

	void Renderer::SetBlendState(BLEND_STATE state)
	{
		static float bFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		s_DeviceContext->OMSetBlendState(s_BlendStates[state], bFactor, 0xffffffff);
	}

	void Renderer::SetRasterizerState(RASTERIZER_STATE state)
	{
		s_DeviceContext->RSSetState(s_RasterizerStates[state]);
	}

	void Renderer::SetViewport(float width, float height)
	{
		D3D11_VIEWPORT viewport;
		viewport.Width = width;
		viewport.Height = height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		s_DeviceContext->RSSetViewports(1, &viewport);
	}

	void Renderer::Apply2D(float viewWidth, float viewHeight)
	{
		XMMATRIX projection = XMMatrixOrthographicOffCenterLH(0.0f, viewWidth, viewHeight, 0.0f, 0.0f, 1.0f);
		projection *= XMMatrixTranslation(1.0f, -1.0f, 0.0f);
		CAMERA_CONSTANT camera{};
		XMStoreFloat4x4(&camera.view, XMMatrixTranspose(XMMatrixIdentity()));
		XMStoreFloat4x4(&camera.projection, XMMatrixTranspose(projection));
		XMStoreFloat4x4(&camera.viewProjection, XMMatrixTranspose(XMMatrixIdentity() * projection));
		Renderer::SetCamera(camera);
	}

	void Renderer::SetCamera(const CAMERA_CONSTANT& constant)
	{
		s_DeviceContext->UpdateSubresource(s_CameraConstantBuffer, 0, NULL, &constant, 0, 0);
	}

	void Renderer::SetLight(const LIGHT_CONSTANT& constant)
	{
		s_DeviceContext->UpdateSubresource(s_LightConstantBuffer, 0, NULL, &constant, 0, 0);
	}

	void Renderer::SetModelContant(const MODEL_CONSTANT& constant)
	{
		s_DeviceContext->UpdateSubresource(s_ModelConstantBuffer, 0, NULL, &constant, 0, 0);
	}

	void Renderer::SetMeshContant(const MESH_CONSTANT& constant)
	{
		s_DeviceContext->UpdateSubresource(s_MeshConstantBuffer, 0, NULL, &constant, 0, 0);
	}

	void Renderer::SetCSContant(const CS_CONSTANT& constant)
	{
		s_DeviceContext->UpdateSubresource(s_CSConstantBuffer, 0, NULL, &constant, 0, 0);
	}

	void Renderer::SetSingleContant(const SINGLE_CONSTANT& constant)
	{
		s_DeviceContext->UpdateSubresource(s_SingleConstantBuffer, 0, NULL, &constant, 0, 0);
	}

	void Renderer::SetParticleContant(const PARTICLE_CONSTANT& constant)
	{
		s_DeviceContext->UpdateSubresource(s_ParticleConstantBuffer, 0, NULL, &constant, 0, 0);
	}

	void Renderer::SetTimeContant(const TIME_CONSTANT& constant)
	{
		s_DeviceContext->UpdateSubresource(s_TimeConstantBuffer, 0, NULL, &constant, 0, 0);
	}

	ID3D11Buffer* Renderer::CreateVertexBuffer(unsigned int byteWidth)
	{
		ID3D11Buffer* newBuffer = nullptr;
		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.ByteWidth = byteWidth;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		Renderer::GetDevice()->CreateBuffer(&desc, nullptr, &newBuffer);
		return newBuffer;
	}

	ID3D11Buffer* Renderer::CreateStructuredBuffer(unsigned int stride, unsigned int count, const void* data, unsigned int bindFlag)
	{
		ID3D11Buffer* newBuffer = nullptr;
		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = bindFlag;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.ByteWidth = stride * count;
		desc.StructureByteStride = stride;

		if (data) {
			D3D11_SUBRESOURCE_DATA subResourceData = {};
			subResourceData.SysMemPitch = 0;
			subResourceData.SysMemSlicePitch = 0;
			subResourceData.pSysMem = data;

			Renderer::GetDevice()->CreateBuffer(&desc, &subResourceData, &newBuffer);
			return newBuffer;
		}

		Renderer::GetDevice()->CreateBuffer(&desc, nullptr, &newBuffer);
		return newBuffer;
	}

	ID3D11ShaderResourceView* Renderer::CreateStructuredSRV(ID3D11Buffer* buffer, unsigned int elementCount)
	{
		ID3D11ShaderResourceView* newSRV = nullptr;
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Buffer.NumElements = elementCount;
		Renderer::GetDevice()->CreateShaderResourceView(buffer, &srvDesc, &newSRV);
		return newSRV;
	}

	ID3D11UnorderedAccessView* Renderer::CreateStructuredUAV(ID3D11Buffer* buffer, unsigned int elementCount, unsigned int flags)
	{
		ID3D11UnorderedAccessView* newUAV = nullptr;
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.Buffer.NumElements = elementCount;
		uavDesc.Buffer.Flags = flags;
		Renderer::GetDevice()->CreateUnorderedAccessView(buffer, &uavDesc, &newUAV);
		return newUAV;
	}

	ID3D11Buffer* Renderer::CreateByteAddressBuffer(unsigned int byteWidth, const void* data, unsigned int bindFlag)
	{
		ID3D11Buffer* newBuffer = nullptr;
		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = bindFlag;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;
		desc.ByteWidth = byteWidth;
		desc.StructureByteStride = 0;

		if (data) {
			D3D11_SUBRESOURCE_DATA subResourceData = {};
			subResourceData.SysMemPitch = 0;
			subResourceData.SysMemSlicePitch = 0;
			subResourceData.pSysMem = data;

			Renderer::GetDevice()->CreateBuffer(&desc, &subResourceData, &newBuffer);
			return newBuffer;
		}

		Renderer::GetDevice()->CreateBuffer(&desc, nullptr, &newBuffer);
		return newBuffer;
	}

	ID3D11ShaderResourceView* Renderer::CreateByteAddressSRV(ID3D11Buffer* buffer, unsigned int byteWidth)
	{
		ID3D11ShaderResourceView* newSRV = nullptr;
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		srvDesc.BufferEx.NumElements = byteWidth / 4;
		srvDesc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
		Renderer::GetDevice()->CreateShaderResourceView(buffer, &srvDesc, &newSRV);
		return newSRV;
	}

	ID3D11UnorderedAccessView* Renderer::CreateByteAddressUAV(ID3D11Buffer* buffer, unsigned int byteWidth)
	{
		ID3D11UnorderedAccessView* newUAV = nullptr;
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		uavDesc.Buffer.NumElements = byteWidth / 4;
		uavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
		Renderer::GetDevice()->CreateUnorderedAccessView(buffer, &uavDesc, &newUAV);
		return newUAV;
	}

	

	ID3D11Texture2D* Renderer::CreateTexture2D(unsigned int width, unsigned int height, DXGI_FORMAT format, unsigned int bindFlags)
	{
		ID3D11Texture2D* texture = nullptr;
		D3D11_TEXTURE2D_DESC texDesc{};
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = format;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = bindFlags;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		s_Device->CreateTexture2D(&texDesc, nullptr, &texture);
		return texture;
	}

	ID3D11RenderTargetView* Renderer::CreateTextureRTV(ID3D11Texture2D* texture, DXGI_FORMAT format)
	{
		ID3D11RenderTargetView* rtv = nullptr;
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

		s_Device->CreateRenderTargetView(texture, &rtvDesc, &rtv);
		return rtv;
	}

	ID3D11ShaderResourceView* Renderer::CreateTextureSRV(ID3D11Texture2D* texture, DXGI_FORMAT format)
	{
		ID3D11ShaderResourceView* srv = nullptr;
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};	
		srvDesc.Format = format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		s_Device->CreateShaderResourceView(texture, &srvDesc, &srv);
		return srv;
	}

	ID3D11DepthStencilView* Renderer::CreateTextureDSV(ID3D11Texture2D* texture, DXGI_FORMAT format)
	{
		ID3D11DepthStencilView* dsv = nullptr;
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = format;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Flags = 0;
		s_Device->CreateDepthStencilView(texture, &dsvDesc, &dsv);
		return dsv;
	}

	void Renderer::SetPrimitiveTopology(PRIMITIVE_TYPE type)
	{
		static std::unordered_map<PRIMITIVE_TYPE, D3D11_PRIMITIVE_TOPOLOGY> mapping = {
			{PRIMITIVE_TYPE_POINT, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST},
			{PRIMITIVE_TYPE_LINE, D3D11_PRIMITIVE_TOPOLOGY_LINELIST},
			{PRIMITIVE_TYPE_TRIANGLE, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST},
			{PRIMITIVE_TYPE_NONE, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP}
		};

		s_DeviceContext->IASetPrimitiveTopology(mapping[type]);

	}

	VERTEX_SHADER_SET Renderer::GetVertexShaderSet(const char* filename)
	{
		if (s_VertexShaderSets.count(filename) > 0) {
			return s_VertexShaderSets[filename];
		}
		if (LoadVertexShader(filename)) {
			return s_VertexShaderSets[filename];
		}
		return {};
	}

	ID3D11PixelShader* Renderer::GetPixelShader(const char* filename)
	{
		if (s_PixelShaders.count(filename) > 0) {
			return s_PixelShaders[filename];
		}
		return LoadPixelShader(filename);
	}

	ID3D11ComputeShader* Renderer::GetComputeShader(const char* filename)
	{
		if (s_ComputeShaders.count(filename) > 0) {
			return s_ComputeShaders[filename];
		}
		return LoadComputeShader(filename);
	}

	ID3D11GeometryShader* Renderer::GetGeometryShader(const char* filename)
	{
		if (s_GeometryShaders.count(filename) > 0) {
			return s_GeometryShaders[filename];
		}
		return LoadGeometryShader(filename);
	}

	ID3D11VertexShader* Renderer::LoadVertexShader(const char* filename)
	{
		ID3D11VertexShader* vertexShader = nullptr;
		std::vector<char> buffer;
		ReadFile(filename, buffer);
		if (buffer.size() != 0) {
			s_Device->CreateVertexShader(buffer.data(), buffer.size(), NULL, &vertexShader);
			s_VertexShaderSets[filename].vertexShader = vertexShader;
			
		}
		return vertexShader;
	}

	ID3D11PixelShader* Renderer::LoadPixelShader(const char* filename)
	{
		ID3D11PixelShader* pixelShader = nullptr;
		std::vector<char> buffer;
		ReadFile(filename, buffer);
		if (buffer.size() != 0) {
			s_Device->CreatePixelShader(buffer.data(), buffer.size(), NULL, &pixelShader);
			s_PixelShaders[filename] = pixelShader;
		}
		return pixelShader;
	}

	ID3D11ComputeShader* Renderer::LoadComputeShader(const char* filename)
	{
		ID3D11ComputeShader* computeShader = nullptr;
		std::vector<char> buffer;
		ReadFile(filename, buffer);
		if (buffer.size() != 0) {
			s_Device->CreateComputeShader(buffer.data(), buffer.size(), NULL, &computeShader);
			s_ComputeShaders[filename] = computeShader;
		}
		return computeShader;
	}

	ID3D11GeometryShader* Renderer::LoadGeometryShader(const char* filename)
	{
		ID3D11GeometryShader* geometryShader = nullptr;
		std::vector<char> buffer;
		ReadFile(filename, buffer);
		if (buffer.size() != 0) {
			s_Device->CreateGeometryShader(buffer.data(), buffer.size(), NULL, &geometryShader);
			s_GeometryShaders[filename] = geometryShader;
		}
		return geometryShader;
	}

	ID3D11InputLayout* Renderer::LoadInputLayout(const char* filename, const char* vertexShaderFilename)
	{
		ID3D11InputLayout* inputLayout = nullptr;
		std::vector<char> layoutBuffer;
		std::vector<char> shaderBuffer;
		ReadFile(filename, layoutBuffer);
		ReadFile(vertexShaderFilename, shaderBuffer);
		if (layoutBuffer.size() != 0 && shaderBuffer.size() != 0) {

			CSVResource csv(reinterpret_cast<unsigned char*>(layoutBuffer.data()), layoutBuffer.size());
			std::vector<D3D11_INPUT_ELEMENT_DESC> layoutElements;
			ParseInputLayout(csv, layoutElements);

			s_Device->CreateInputLayout(layoutElements.data(),
				static_cast<UINT>(layoutElements.size()),
				shaderBuffer.data(),
				shaderBuffer.size(),
				&inputLayout);
			s_VertexShaderSets[vertexShaderFilename].inputLayouts[filename] = inputLayout;
		}
		return inputLayout;
	}

	void Renderer::SetMainRenderTarget()
	{
		s_DeviceContext->OMSetRenderTargets(1, &s_RenderTargetView, s_DepthStencilView);
	}

	D3D11_INPUT_CLASSIFICATION Renderer::GetInputClassByName(const char* name)
	{
		static std::unordered_map<std::string, D3D11_INPUT_CLASSIFICATION> inputClasses{
			{ "D3D11_INPUT_PER_VERTEX_DATA", D3D11_INPUT_PER_VERTEX_DATA },
			{ "D3D11_INPUT_PER_INSTANCE_DATA", D3D11_INPUT_PER_INSTANCE_DATA }
		};
		if (inputClasses.count(name) > 0) {
			return inputClasses[name];
		}
		return D3D11_INPUT_PER_VERTEX_DATA;
	}

	DXGI_FORMAT Renderer::GetDXGIFormatByName(const char* name)
	{
		static std::unordered_map<std::string, DXGI_FORMAT> formats{
			{ "DXGI_FORMAT_UNKNOWN", DXGI_FORMAT_UNKNOWN },
			{ "DXGI_FORMAT_R32G32B32A32_TYPELESS", DXGI_FORMAT_R32G32B32A32_TYPELESS },
			{ "DXGI_FORMAT_R32G32B32A32_FLOAT",    DXGI_FORMAT_R32G32B32A32_FLOAT },
			{ "DXGI_FORMAT_R32G32B32A32_UINT",     DXGI_FORMAT_R32G32B32A32_UINT },
			{ "DXGI_FORMAT_R32G32B32A32_SINT",     DXGI_FORMAT_R32G32B32A32_SINT },
			{ "DXGI_FORMAT_R32G32B32_TYPELESS", DXGI_FORMAT_R32G32B32_TYPELESS },
			{ "DXGI_FORMAT_R32G32B32_FLOAT",    DXGI_FORMAT_R32G32B32_FLOAT },
			{ "DXGI_FORMAT_R32G32B32_UINT",     DXGI_FORMAT_R32G32B32_UINT },
			{ "DXGI_FORMAT_R32G32B32_SINT",     DXGI_FORMAT_R32G32B32_SINT },
			{ "DXGI_FORMAT_R16G16B16A16_TYPELESS", DXGI_FORMAT_R16G16B16A16_TYPELESS },
			{ "DXGI_FORMAT_R16G16B16A16_FLOAT",    DXGI_FORMAT_R16G16B16A16_FLOAT },
			{ "DXGI_FORMAT_R16G16B16A16_UNORM",    DXGI_FORMAT_R16G16B16A16_UNORM },
			{ "DXGI_FORMAT_R16G16B16A16_UINT",     DXGI_FORMAT_R16G16B16A16_UINT },
			{ "DXGI_FORMAT_R16G16B16A16_SNORM",    DXGI_FORMAT_R16G16B16A16_SNORM },
			{ "DXGI_FORMAT_R16G16B16A16_SINT",     DXGI_FORMAT_R16G16B16A16_SINT },
			{ "DXGI_FORMAT_R32G32_TYPELESS", DXGI_FORMAT_R32G32_TYPELESS },
			{ "DXGI_FORMAT_R32G32_FLOAT",    DXGI_FORMAT_R32G32_FLOAT },
			{ "DXGI_FORMAT_R32G32_UINT",     DXGI_FORMAT_R32G32_UINT },
			{ "DXGI_FORMAT_R32G32_SINT",     DXGI_FORMAT_R32G32_SINT },
			{ "DXGI_FORMAT_R32_TYPELESS", DXGI_FORMAT_R32_TYPELESS },
			{ "DXGI_FORMAT_R32_FLOAT",    DXGI_FORMAT_R32_FLOAT },
			{ "DXGI_FORMAT_R32_UINT",     DXGI_FORMAT_R32_UINT },
			{ "DXGI_FORMAT_R32_SINT",     DXGI_FORMAT_R32_SINT },
			{ "DXGI_FORMAT_R16_TYPELESS", DXGI_FORMAT_R16_TYPELESS },
			{ "DXGI_FORMAT_R16_FLOAT",    DXGI_FORMAT_R16_FLOAT },
			{ "DXGI_FORMAT_R16_UNORM",    DXGI_FORMAT_R16_UNORM },
			{ "DXGI_FORMAT_R16_UINT",     DXGI_FORMAT_R16_UINT },
			{ "DXGI_FORMAT_R16_SNORM",    DXGI_FORMAT_R16_SNORM },
			{ "DXGI_FORMAT_R16_SINT",     DXGI_FORMAT_R16_SINT },
			{ "DXGI_FORMAT_R8_TYPELESS", DXGI_FORMAT_R8_TYPELESS },
			{ "DXGI_FORMAT_R8_UNORM",    DXGI_FORMAT_R8_UNORM },
			{ "DXGI_FORMAT_R8_UINT",     DXGI_FORMAT_R8_UINT },
			{ "DXGI_FORMAT_R8_SNORM",    DXGI_FORMAT_R8_SNORM },
			{ "DXGI_FORMAT_R8_SINT",     DXGI_FORMAT_R8_SINT },
			{ "DXGI_FORMAT_A8_UNORM", DXGI_FORMAT_A8_UNORM },
			{ "DXGI_FORMAT_B8G8R8A8_UNORM",       DXGI_FORMAT_B8G8R8A8_UNORM },
			{ "DXGI_FORMAT_B8G8R8A8_TYPELESS",    DXGI_FORMAT_B8G8R8A8_TYPELESS },
			{ "DXGI_FORMAT_B8G8R8A8_UNORM_SRGB",  DXGI_FORMAT_B8G8R8A8_UNORM_SRGB },
			{ "DXGI_FORMAT_FORCE_UINT", DXGI_FORMAT_FORCE_UINT },
		};

		if (formats.count(name) > 0) {
			return formats[name];
		}
		return DXGI_FORMAT_UNKNOWN;
	}
} // namespace MG


