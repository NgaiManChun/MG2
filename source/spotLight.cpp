#include "spotLight.h"
#include "renderer.h"
#include "input.h"
#include "scene.h"
#include "MGUtility.h"
#include "modelRenderer.h"

namespace MG {


	void SpotLight::Init()
	{
		
	}

	void SpotLight::StaticInit()
	{
		
	}

	void SpotLight::StaticUninit()
	{

	}

	void SpotLight::UpdateAll(Scene* scene)
	{
		auto& component_pair = Component::s_Components<SpotLight>[scene];
		auto& sceneComponents = component_pair.components;
		size_t& destoryedComponentIndex = component_pair.destoryedComponentIndex;
		size_t capacity = sceneComponents.capacity();

		if (capacity > s_Capcity) {
			for (ID3D11DepthStencilView* dsv : s_DSVs) {
				SAFE_RELEASE(dsv);
			}
			SAFE_RELEASE(s_ArraySRV);
			SAFE_RELEASE(s_shadowTexArray);

			unsigned int textureSize = max(MGUtility::GetScreenWidth(), MGUtility::GetScreenHeight()) / 4;

			D3D11_TEXTURE2D_DESC texDesc = {};
			texDesc.Width = textureSize;
			texDesc.Height = textureSize;
			texDesc.MipLevels = 1;
			texDesc.ArraySize = capacity;
			texDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.Usage = D3D11_USAGE_DEFAULT;
			texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
			texDesc.CPUAccessFlags = 0;
			texDesc.MiscFlags = 0;

			Renderer::GetDevice()->CreateTexture2D(&texDesc, nullptr, &s_shadowTexArray);

			s_DSVs.resize(capacity);
			for (UINT i = 0; i < capacity; i++)
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
				dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
				dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				dsvDesc.Texture2DArray.ArraySize = 1;
				dsvDesc.Texture2DArray.MipSlice = 0;
				dsvDesc.Texture2DArray.FirstArraySlice = i;

				Renderer::GetDevice()->CreateDepthStencilView(
					s_shadowTexArray,
					&dsvDesc,
					&s_DSVs[i]
				);
				Renderer::GetDeviceContext()->ClearDepthStencilView(s_DSVs[i], D3D11_CLEAR_DEPTH, 1.0f, 0);
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.Texture2DArray.MipLevels = 1;
			srvDesc.Texture2DArray.MostDetailedMip = 0;
			srvDesc.Texture2DArray.FirstArraySlice = 0;
			srvDesc.Texture2DArray.ArraySize = capacity;

			Renderer::GetDevice()->CreateShaderResourceView(
				s_shadowTexArray,
				&srvDesc,
				&s_ArraySRV
			);

			s_Capcity = capacity;
		}

	}

	void SpotLight::DrawAll(Scene* scene)
	{
		ID3D11Device* device = Renderer::GetDevice();
		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();

		static float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		// ビューポート設定
		float viewSize = max(MGUtility::GetScreenWidth(), MGUtility::GetScreenHeight()) / 4;
		Renderer::SetViewport(viewSize, viewSize);

		// シェーダ設定
		auto shaderSet = Renderer::GetShaderSet(SHADER_TYPE_SPOT_LIGHT);
		deviceContext->VSSetShader(shaderSet.vertexShader, NULL, 0);
		deviceContext->PSSetShader(shaderSet.pixelShader, NULL, 0);
		deviceContext->IASetInputLayout(shaderSet.inputLayout);

		/*Renderer::SetRasterizerState(RASTERIZER_STATE_SOLID_CULL_BACK);
		Renderer::SetDepthState(DEPTH_STATE_COMPARISON_LESS);*/

		static std::vector<SPOT_LIGHT> spotLightMetaArray;
		spotLightMetaArray.clear();

		auto& component_pair = Component::s_Components<SpotLight>[scene];
		auto& sceneComponents = component_pair.components;
		size_t& destoryedComponentIndex = component_pair.destoryedComponentIndex;
		size_t size = sceneComponents.size();
		for (ptrdiff_t i = static_cast<ptrdiff_t>(size) - 1; i >= 0; i--) {
			if (sceneComponents[i]) {
				SpotLight& component = *sceneComponents[i];
				component.Initialize();
				if (component.IsEnabled()) {

					deviceContext->ClearDepthStencilView(s_DSVs[i], D3D11_CLEAR_DEPTH, 1.0f, 0);
					deviceContext->OMSetRenderTargets(0, nullptr, s_DSVs[i]);
					component.Apply();
					
					ModelRenderer::Culling();

					ModelRenderer::DrawAll(scene);

					component.m_Data.distance = component.m_Far;
					component.m_Data.color = component.m_Color;
					component.m_Data.blur = 1.0f;
					component.m_Data.enabled = true;
					spotLightMetaArray.push_back(component.m_Data);
				}
				else if (component.IsDestroyed()) {
					destoryedComponentIndex = i;
				}
			}
		}

		
	}

	void SpotLight::Apply()
	{
		GameObject* gameObject = GetGameObject();
		XMVECTOR position = gameObject->GetPosition();
		XMVECTOR forward = gameObject->GetForward();
		XMMATRIX view = XMMatrixLookAtLH(position, position + forward, gameObject->GetUpper());

		XMMATRIX projection = XMMatrixPerspectiveFovLH(m_Angle, MGUtility::GetScreenRatio(), m_Near, m_Far);

		XMMATRIX invViewRotation = XMMatrixInverse(nullptr, view);
		invViewRotation.r[3].m128_f32[0] = 0.0f;
		invViewRotation.r[3].m128_f32[1] = 0.0f;
		invViewRotation.r[3].m128_f32[2] = 0.0f;

		XMMATRIX invVp = XMMatrixInverse(nullptr, view * projection);

		Vector3 points[4] = {
			{ -1.0f,  1.0f, 1.0f },
			{  1.0f,  1.0f, 1.0f },
			{ -1.0f, -1.0f, 1.0f },
			{  1.0f, -1.0f, 1.0f }
		};
		points[0] = XMVector3TransformCoord(points[0], invVp);
		points[1] = XMVector3TransformCoord(points[1], invVp);
		points[2] = XMVector3TransformCoord(points[2], invVp);
		points[3] = XMVector3TransformCoord(points[3], invVp);

		m_Frustum = {
			XMVector3Normalize(XMVector3Cross(points[0] - position, points[2] - position)),
			XMVector3Normalize(XMVector3Cross(points[3] - position, points[1] - position)),
			XMVector3Normalize(XMVector3Cross(points[1] - position, points[0] - position)),
			XMVector3Normalize(XMVector3Cross(points[2] - position, points[3] - position)),
			forward
		};

		CAMERA_CONSTANT camera = {};
		XMStoreFloat4x4(&camera.view, XMMatrixTranspose(view));
		XMStoreFloat4x4(&camera.projection, XMMatrixTranspose(projection));
		XMStoreFloat4x4(&camera.viewProjection, XMMatrixTranspose(view * projection));
		XMStoreFloat4x4(&camera.invViewProjection, XMMatrixTranspose(XMMatrixInverse(nullptr, view * projection)));
		XMStoreFloat4x4(&camera.invViewRotation, XMMatrixTranspose(invViewRotation));
		camera.cameraPosition = gameObject->GetPosition();
		camera.projectionFar = m_Far;
		memcpy(camera.frustum, m_Frustum.data(), sizeof(Vector4) * 5);
		Renderer::SetCamera(camera);

		m_Data.viewProjection = camera.viewProjection;
		m_Data.position = position;
		m_Data.forward = forward;
		
	}
} // namespace MG


