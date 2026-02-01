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
		GameObject* gameObject = GetGameObject();
		Camera* camera = gameObject->GetScene()->GetMainCamera();
		gameObject->SetPosition(camera->GetPosition());
		DynamicMatrix::Update();
		camera->Apply();

		static SHADER_SET shaderSet = ([this]() -> SHADER_SET {
			D3D11_INPUT_ELEMENT_DESC layout[] =
			{
				{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0,  0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL",			0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TANGENT"	,		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 24,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "BINORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 36,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		0, 48,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR",			0, DXGI_FORMAT_R32G32B32A32_FLOAT,	0, 56,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			auto shaderSet = Renderer::LoadVertexShader("complied_shader\\skyVS.cso", layout, ARRAYSIZE(layout));
			shaderSet.pixelShader = Renderer::LoadPixelShader("complied_shader\\testPS.cso");
			return shaderSet;
		})();

		

		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();
		deviceContext->VSSetShader(shaderSet.vertexShader, NULL, 0);
		deviceContext->IASetInputLayout(shaderSet.inputLayout);
		deviceContext->PSSetShader(shaderSet.pixelShader, NULL, 0);

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

		unsigned int originalNodeMatrixDivisionOffset = modelData.originalNodeMatrixDivision.GetMetaData().offset;

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
				materialData.opacityTexture.GetSRV()
			};
			deviceContext->PSSetShaderResources(0, ARRAYSIZE(textureSRVs), textureSRVs);
			
			auto& vertexIndexData = meshData.vertexIndexDivision.GetMetaData();

			deviceContext->DrawIndexed(vertexIndexData.count, vertexIndexData.offset, meshData.vertexDivision.GetMetaData().offset);

		}

		Renderer::SetRasterizerState(RASTERIZER_STATE_SOLID_CULL_BACK);

	}

} // namespace MG

