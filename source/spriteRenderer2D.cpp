#include "spriteRenderer2D.h"
#include "renderer.h"
#include "MGUtility.h"

namespace MG {

	void SpriteRenderer2D::DrawAll(Scene* scene)
	{
		auto& component_pair = Component::s_Components<SpriteRenderer2D>[scene];
		auto& sceneComponents = component_pair.components;
		size_t& destoryedComponentIndex = component_pair.destoryedComponentIndex;
		size_t size = sceneComponents.size();

		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();

		static SHADER_SET shaderSet = ([]() {
			SHADER_SET shaderSet = Renderer::LoadVertexShader("complied_shader\\spriteVS.cso", nullptr, 0);
			shaderSet.pixelShader = Renderer::LoadPixelShader("complied_shader\\unlitTexturePS.cso");
			return shaderSet;
			})();

		deviceContext->VSSetShader(shaderSet.vertexShader, NULL, 0);
		deviceContext->IASetInputLayout(shaderSet.inputLayout);
		deviceContext->PSSetShader(shaderSet.pixelShader, NULL, 0);

		Renderer::Apply2D(
			static_cast<float>(MGUtility::GetScreenWidth()), 
			static_cast<float>(MGUtility::GetScreenHeight())
		);

		ID3D11ShaderResourceView* srvArray[] = {
			DynamicMatrix::GetSRV()
		};
		deviceContext->VSSetShaderResources(0, ARRAYSIZE(srvArray), srvArray);

		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		Renderer::SetDepthState(DEPTH_STATE_NO_WRITE_COMPARISON_ALWAYS);
		Renderer::SetMainRenderTarget();
		for (ptrdiff_t i = static_cast<ptrdiff_t>(size) - 1; i >= 0; i--) {
			if (sceneComponents[i] && sceneComponents[i]->IsEnabled()) {
				SpriteRenderer2D& component = *sceneComponents[i];

				SINGLE_CONSTANT singleConstant{};
				singleConstant.worldMatrixId = component.GetGameObject()->GetWorldMatrix();
				singleConstant.materialId = component.m_Material;
				Renderer::SetSingleContant(singleConstant);

				// ƒ}ƒeƒŠƒAƒ‹
				{
					auto& materialData = component.m_Material.GetData();
					ID3D11ShaderResourceView* srvArray[] = {
						materialData.baseTexture.GetSRV(),
						materialData.normalTexture.GetSRV(),
						materialData.opacityTexture.GetSRV(),
						Material::GetSRV()
					};
					deviceContext->PSSetShaderResources(0, ARRAYSIZE(srvArray), srvArray);
				}

				deviceContext->Draw(4, 0);
			}
		}
	}

	/*void SpriteRenderer2D::Draw()
	{

		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();

		static SHADER_SET shaderSet = ([]() {
			SHADER_SET shaderSet = Renderer::LoadVertexShader("complied_shader\\spriteVS.cso", nullptr, 0);
			shaderSet.pixelShader = Renderer::LoadPixelShader("complied_shader\\unlitTexturePS.cso");
			return shaderSet;
		})();
	}*/

} // namespace MG

