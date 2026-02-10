#include "spriteRenderer2D.h"
#include "renderer.h"
#include "MGUtility.h"

namespace MG {

	void SpriteRenderer2D::DrawAll(Scene* scene, std::vector<SpriteRenderer2D*>& components)
	{
		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();

		// シェーダ
		static auto vertexShaderSet = Renderer::GetVertexShaderSet("VS/spriteVS.cso");
		static ID3D11PixelShader* pixelShader = Renderer::GetPixelShader("PS/unlitTexturePS.cso");
		deviceContext->VSSetShader(vertexShaderSet.vertexShader, NULL, 0);
		deviceContext->IASetInputLayout(nullptr);
		deviceContext->PSSetShader(pixelShader, NULL, 0);

		// SRV
		ID3D11ShaderResourceView* srvArray[] = {
			DynamicMatrix::GetSRV()
		};
		deviceContext->VSSetShaderResources(0, ARRAYSIZE(srvArray), srvArray);

		// ※TRIANGLESTRIP
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		// デプスoff
		Renderer::SetDepthState(DEPTH_STATE_NO_WRITE_COMPARISON_ALWAYS);

		Renderer::SetMainRenderTarget();

		// 2Dカメラ設定
		Renderer::Apply2D(
			static_cast<float>(MGUtility::GetScreenWidth()),
			static_cast<float>(MGUtility::GetScreenHeight())
		);

		for (auto component : components) {
			if (component && component->IsActive()) {

				// シングルメッシュ定数
				SINGLE_CONSTANT singleConstant{};
				singleConstant.worldMatrixId = component->GetGameObject()->GetWorldMatrix();
				singleConstant.materialId = component->m_Material;
				Renderer::SetSingleContant(singleConstant);

				// マテリアル
				auto& materialData = component->m_Material.GetData();
				ID3D11ShaderResourceView* srvArray[] = {
					materialData.baseTexture.GetSRV(),
					materialData.normalTexture.GetSRV(),
					materialData.opacityTexture.GetSRV(),
					Material::GetSRV()
				};
				deviceContext->PSSetShaderResources(0, ARRAYSIZE(srvArray), srvArray);

				deviceContext->Draw(4, 0);
			}
		}
	}

} // namespace MG

