#include "skyRenderer.h"
#include "renderer.h"
#include "dynamicMatrix.h"
#include "matrixDivision.h"
#include "MGUtility.h"
#include "camera.h"


namespace MG {

	void SkyRenderer::StaticInit()
	{/*
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",			0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT"	,		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 24,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 36,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		0, 48,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",			0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 56,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		s_ShaderSet = Renderer::LoadVertexShader("complied_shader\\skyVS.cso", layout, ARRAYSIZE(layout));
		s_ShaderSet.pixelShader = Renderer::LoadPixelShader("complied_shader\\unlitTexturePS.cso");*/
	}

	void SkyRenderer::StaticUninit()
	{

	}

	void SkyRenderer::UpdateAll(Scene* scene)
	{
		DynamicMatrix::Update();

	}

	void SkyRenderer::Update()
	{
		
	}

	void SkyRenderer::Draw()
	{
		if (!m_Model)
			return;

		GameObject* gameObject = GetGameObject();
		Camera* camera = gameObject->GetScene()->GetMainCamera();
		gameObject->SetPosition(camera->GetPosition());
		DynamicMatrix::Update();
		camera->Apply();

		static auto vertexShaderSet = Renderer::GetVertexShaderSet("VS/modelSimpleVS.cso");
		static ID3D11InputLayout* inputLayout = vertexShaderSet.inputLayouts["general.csv"];
		static ID3D11PixelShader* pixelShader = Renderer::GetPixelShader("PS/unlitTexturePS.cso");

		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();
		deviceContext->VSSetShader(vertexShaderSet.vertexShader, NULL, 0);
		deviceContext->IASetInputLayout(inputLayout);
		deviceContext->PSSetShader(pixelShader, NULL, 0);

		UINT strides[] = {
			sizeof(VERTEX)
		};
		UINT offsets[] = { 0 };
		ID3D11Buffer* vertexBuffers[] = {
			VertexDivision::GetDataBuffer()
		};

		// インデックスバッファ設定
		deviceContext->IASetIndexBuffer(VertexIndexDivision::GetDataBuffer(), DXGI_FORMAT_R32_UINT, 0);
		deviceContext->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);

		Renderer::SetMainRenderTarget();
		Renderer::SetViewport(MGUtility::GetScreenWidth(), MGUtility::GetScreenHeight());
		


		ID3D11ShaderResourceView* srvArray[] = {
			DynamicMatrix::GetSRV(),
			MatrixDivision::GetDataSRV()
		};
		deviceContext->VSSetShaderResources(0, ARRAYSIZE(srvArray), srvArray);
		Renderer::SetRasterizerState(RASTERIZER_STATE_SOLID_CULL_NONE);
		Renderer::SetDepthState(DEPTH_STATE_NO_WRITE_COMPARISON_ALWAYS);

		auto& modelData = m_Model.GetData();

		SINGLE_CONSTANT constant{};
		constant.worldMatrixId = GetGameObject()->GetWorldMatrix();

		unsigned int originalNodeMatrixDivisionOffset = modelData.originalNodeMatrixDivision.GetBookmarkData().offset;

		for (auto& pair : modelData.nodeMeshPairs) {
			Mesh mesh = modelData.meshes[pair.meshOffset];
			auto& meshData = mesh.GetData();
			unsigned int nodeIndex = pair.nodeOffset;
			Material material = modelData.materials[meshData.materialOffset];
			auto materialData = material.GetData();


			constant.localMatrixDivisionOffset = originalNodeMatrixDivisionOffset + nodeIndex;
			constant.materialId = material;
			Renderer::SetSingleContant(constant);

			ID3D11ShaderResourceView* textureSRVs[] = {
				materialData.baseTexture.GetSRV(),
				materialData.normalTexture.GetSRV(),
				materialData.opacityTexture.GetSRV(),
				Material::GetSRV()
			};
			deviceContext->PSSetShaderResources(0, ARRAYSIZE(textureSRVs), textureSRVs);
			
			auto& vertexIndexData = meshData.vertexIndexDivision.GetBookmarkData();

			deviceContext->DrawIndexed(vertexIndexData.count, vertexIndexData.offset, meshData.vertexDivision.GetBookmarkData().offset);

		}

		Renderer::SetRasterizerState(RASTERIZER_STATE_SOLID_CULL_BACK);

	}

} // namespace MG

