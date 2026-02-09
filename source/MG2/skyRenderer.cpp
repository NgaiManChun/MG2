#include "skyRenderer.h"
#include "renderer.h"
#include "dynamicMatrix.h"
#include "matrixDivision.h"
#include "MGUtility.h"
#include "camera.h"

namespace MG {

	void SkyRenderer::Update()
	{
		GameObject* gameObject = GetGameObject();
		Camera* camera = gameObject->GetScene()->GetMainCamera();
		gameObject->SetPosition(camera->GetPosition());

		DynamicMatrix::Update();
	}

	void SkyRenderer::Draw()
	{
		if (!m_Model)
			return;

		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();

		// シェーダ
		static auto vertexShaderSet = Renderer::GetVertexShaderSet("VS/modelSimpleVS.cso");
		static ID3D11InputLayout* inputLayout = vertexShaderSet.inputLayouts["general.csv"];
		static ID3D11PixelShader* pixelShader = Renderer::GetPixelShader("PS/unlitTexturePS.cso");
		deviceContext->VSSetShader(vertexShaderSet.vertexShader, NULL, 0);
		deviceContext->IASetInputLayout(inputLayout);
		deviceContext->PSSetShader(pixelShader, NULL, 0);

		// 頂点バッファ
		UINT strides[] = { sizeof(VERTEX) };
		UINT offsets[] = { 0 };
		ID3D11Buffer* vertexBuffers[] = { VertexDivision::GetDataBuffer() };
		deviceContext->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);

		// 頂点インデックスバッファ
		deviceContext->IASetIndexBuffer(VertexIndexDivision::GetDataBuffer(), DXGI_FORMAT_R32_UINT, 0);
		
		// レンダーターゲット
		Renderer::SetMainRenderTarget();

		// ビューポート
		Renderer::SetViewport(
			static_cast<float>(MGUtility::GetScreenWidth()), 
			static_cast<float>(MGUtility::GetScreenHeight())
		);
		
		// SRV
		ID3D11ShaderResourceView* srvArray[] = {
			DynamicMatrix::GetSRV(),
			MatrixDivision::GetDataSRV()
		};
		deviceContext->VSSetShaderResources(0, ARRAYSIZE(srvArray), srvArray);

		// カーリングoff
		Renderer::SetRasterizerState(RASTERIZER_STATE_SOLID_CULL_NONE);

		// デプスoff
		Renderer::SetDepthState(DEPTH_STATE_NO_WRITE_COMPARISON_ALWAYS);

		// シングルメッシュ定数
		SINGLE_CONSTANT constant{};
		constant.worldMatrixId = GetGameObject()->GetWorldMatrix();

		// カメラ適用
		GameObject* gameObject = GetGameObject();
		Camera* camera = gameObject->GetScene()->GetMainCamera();
		camera->Apply();

		auto& modelData = m_Model.GetData();
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

			// テクスチャ
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

		// カーリング設定を元に戻す
		Renderer::SetRasterizerState(RASTERIZER_STATE_SOLID_CULL_BACK);

	}

} // namespace MG

