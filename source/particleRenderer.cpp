#include "particleRenderer.h"
#include "renderer.h"
#include "input.h"
#include "scene.h"
#include "camera.h"
#include "MGUtility.h"

namespace MG {

	ParticleRenderer::ParticleRenderer(Material material, size_t count, const char* updateCSFilename)
		: m_Material(material), m_Count(count)
	{
		if (s_CSPool.count(updateCSFilename) > 0) {
			m_UpdateCS = s_CSPool[updateCSFilename];
		}
		else {
			m_UpdateCS = Renderer::LoadComputeShader(updateCSFilename);
			s_CSPool[updateCSFilename] = m_UpdateCS;
		}

		DRAW_INDIRECT_ARGS drawArgs{};
		drawArgs.vertexCountPerInstance = 4;
		drawArgs.instanceCount = 0;
		drawArgs.startVertexLocation = 0;
		drawArgs.startInstanceLocation = 0;
		drawArgs.instanceMaxCount = m_Count;

		if (s_EmptyIds.empty()) {
			s_DrawArgs.push_back(drawArgs);
			m_ArgsIndex = s_DrawArgs.size() - 1;
		}
		else {
			m_ArgsIndex = *s_EmptyIds.begin();
			s_EmptyIds.erase(s_EmptyIds.begin());
			s_DrawArgs[m_ArgsIndex] = drawArgs;
		}
	}

	void ParticleRenderer::Init()
	{
		

		m_DataBuffer = Renderer::CreateStructuredBuffer(sizeof(PARTICLE), m_Count, nullptr, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
		m_DataSRV = Renderer::CreateStructuredSRV(m_DataBuffer, m_Count);
		m_DataUAV = Renderer::CreateStructuredUAV(m_DataBuffer, m_Count);

		m_IndexBuffer = Renderer::CreateStructuredBuffer(sizeof(unsigned int), m_Count, nullptr, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
		m_IndexSRV = Renderer::CreateStructuredSRV(m_IndexBuffer, m_Count);
		m_IndexUAV = Renderer::CreateStructuredUAV(m_IndexBuffer, m_Count, D3D11_BUFFER_UAV_FLAG_APPEND);
		

	}

	void ParticleRenderer::Uninit()
	{
		SAFE_RELEASE(m_IndexSRV);
		SAFE_RELEASE(m_IndexUAV);
		SAFE_RELEASE(m_DataSRV);
		SAFE_RELEASE(m_DataUAV);
		SAFE_RELEASE(m_IndexBuffer);
		SAFE_RELEASE(m_DataBuffer);
		s_EmptyIds.insert(m_ArgsIndex);
		m_ArgsIndex = UINT_MAX;
	}

	void ParticleRenderer::StaticInit()
	{
		s_SpawnCountBuffer = Renderer::CreateStructuredBuffer(sizeof(int), 1, nullptr, D3D11_BIND_UNORDERED_ACCESS);
		s_SpawnCountUAV = Renderer::CreateStructuredUAV(s_SpawnCountBuffer, 1);
	}
	void ParticleRenderer::StaticUninit()
	{
		for (auto pair : s_CSPool) {
			SAFE_RELEASE(pair.second);
		}
		SAFE_RELEASE(s_DrawArgsSRV);
		SAFE_RELEASE(s_DrawArgsUAV);
		SAFE_RELEASE(s_DrawArgsBuffer);
		SAFE_RELEASE(s_DrawArgsIndirectBuffer);
		s_CSPool.clear();
		s_EmptyIds.clear();
		s_DrawArgs.clear();
		s_Capcity = 0;
	}
	void ParticleRenderer::UpdateAll(Scene* scene)
	{
		scene->UpdateGameObjectWorlds();
		DynamicMatrix::Update();
		Material::Update();

		auto& component_pair = Component::s_Components<ParticleRenderer>[scene];
		auto& sceneComponents = component_pair.components;
		size_t& destoryedComponentIndex = component_pair.destoryedComponentIndex;
		size_t size = s_DrawArgs.size(); //sceneComponents.size();
		size_t capcity = s_DrawArgs.capacity(); // sceneComponents.capacity();

		if (capcity > s_Capcity) {
			SAFE_RELEASE(s_DrawArgsSRV);
			SAFE_RELEASE(s_DrawArgsUAV);
			SAFE_RELEASE(s_DrawArgsBuffer);
			SAFE_RELEASE(s_DrawArgsIndirectBuffer);

			s_DrawArgsBuffer = Renderer::CreateStructuredBuffer(sizeof(DRAW_INDIRECT_ARGS), capcity, s_DrawArgs.data(), D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS);
			s_DrawArgsUAV = Renderer::CreateStructuredUAV(s_DrawArgsBuffer, capcity);
			s_DrawArgsSRV = Renderer::CreateStructuredSRV(s_DrawArgsBuffer, capcity);

			D3D11_BUFFER_DESC desc = {};
			desc.ByteWidth = sizeof(DRAW_INDIRECT_ARGS) * capcity;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = 0;
			desc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
			D3D11_SUBRESOURCE_DATA subResourceData = {};
			subResourceData.SysMemPitch = 0;
			subResourceData.SysMemSlicePitch = 0;
			subResourceData.pSysMem = s_DrawArgs.data();
			Renderer::GetDevice()->CreateBuffer(&desc, &subResourceData, &s_DrawArgsIndirectBuffer);
			s_Capcity = capcity;
		}

		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();

		// カウンターをリセットする
		{
			ID3D11UnorderedAccessView* uavArray[] = {
				nullptr,
				s_DrawArgsUAV
			};
			deviceContext->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavArray), uavArray, nullptr);

			static ID3D11ComputeShader* resetInstanceCountCS = Renderer::LoadComputeShader("complied_shader\\resetInstanceCountCS.cso");
			deviceContext->CSSetShader(resetInstanceCountCS, NULL, 0);

			CS_CONSTANT constant{};
			constant.CSMaxX = s_Capcity;
			Renderer::SetCSContant(constant);
			deviceContext->Dispatch(static_cast<UINT>(ceil((float)constant.CSMaxX / 64)), 1, 1);
		}

		deviceContext->CSSetUnorderedAccessViews(0, 1, &s_DrawArgsUAV, nullptr);

		float deltaTime = MGUtility::GetDeltaTime();
		unsigned int nowTime = timeGetTime();

		for (ptrdiff_t i = static_cast<ptrdiff_t>(size) - 1; i >= 0; i--) {
			if (sceneComponents[i]) {
				ParticleRenderer& component = *sceneComponents[i];
				component.Initialize();
				if (component.IsEnabled()) {
					component.m_UpdateCS;
					UINT count = 0;

					int spawnCount = 0;
					if (component.m_SpawnTimeLine == 1.0f) {
						spawnCount = component.m_SpawnCount;
						component.m_SpawnTimeLine.SetCurrent(0.0f);
					}
					deviceContext->UpdateSubresource(s_SpawnCountBuffer, 0, nullptr, &spawnCount, 0, 0);
					ID3D11UnorderedAccessView* uavArray[] = {
						component.m_DataUAV,
						s_SpawnCountUAV
					};
					deviceContext->CSSetUnorderedAccessViews(1, ARRAYSIZE(uavArray), uavArray, nullptr);
					deviceContext->CSSetUnorderedAccessViews(3, 1, &component.m_IndexUAV, &count);

					CS_CONSTANT constant{};
					constant.CSMaxX = component.m_Count;
					constant.CSMaxY = component.m_ArgsIndex;
					constant.CSMaxZ = nowTime;
					memcpy(&constant.CSMaxW, &deltaTime, sizeof(float));
					Renderer::SetCSContant(constant);
					
					Renderer::SetParticleContant(component.m_Constant);

					deviceContext->CSSetShader(component.m_UpdateCS, nullptr, 0);

					deviceContext->Dispatch(static_cast<UINT>(ceil((float)constant.CSMaxX / 64)), 1, 1);

					

					component.m_SpawnTimeLine.IncreaseValue(deltaTime);
					
				}
			}
		}
	}
	void ParticleRenderer::DrawAll(Scene* scene)
	{
		auto& component_pair = Component::s_Components<ParticleRenderer>[scene];
		auto& sceneComponents = component_pair.components;
		size_t& destoryedComponentIndex = component_pair.destoryedComponentIndex;
		size_t size = sceneComponents.size();

		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();

		static SHADER_SET shaderSet = ([]() {
			SHADER_SET shaderSet = Renderer::LoadVertexShader("complied_shader\\particleVS.cso", nullptr, 0);
			shaderSet.pixelShader = Renderer::LoadPixelShader("complied_shader\\unlitTexturePS.cso");
			return shaderSet;
		})();
		
		deviceContext->VSSetShader(shaderSet.vertexShader, NULL, 0);
		deviceContext->IASetInputLayout(shaderSet.inputLayout);
		deviceContext->PSSetShader(shaderSet.pixelShader, NULL, 0);

		scene->GetMainCamera()->Apply();

		deviceContext->CopyResource(s_DrawArgsIndirectBuffer, s_DrawArgsBuffer);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		Renderer::SetDepthState(DEPTH_STATE_NO_WRITE_COMPARISON_LESS);
		Renderer::SetMainRenderTarget();
		for (ptrdiff_t i = static_cast<ptrdiff_t>(size) - 1; i >= 0; i--) {
			if (sceneComponents[i] && sceneComponents[i]->IsEnabled()) {
				ParticleRenderer& component = *sceneComponents[i];

				SINGLE_CONSTANT singleConstant{};
				singleConstant.worldMatrixId = component.GetGameObject()->GetWorldMatrix();
				singleConstant.materialId = component.m_Material;
				Renderer::SetSingleContant(singleConstant);

				{
					ID3D11UnorderedAccessView* uavArray[] = {
							nullptr,
							nullptr,
							nullptr,
							nullptr,
							nullptr
					};
					deviceContext->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavArray), uavArray, nullptr);
					ID3D11ShaderResourceView* srvArray[] = {
						component.m_DataSRV,
						component.m_IndexSRV,
						DynamicMatrix::GetSRV()
					};
					deviceContext->VSSetShaderResources(0, ARRAYSIZE(srvArray), srvArray);
				}

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
				unsigned int offset = sizeof(DRAW_INDIRECT_ARGS) * component.m_ArgsIndex;
				deviceContext->DrawInstancedIndirect(s_DrawArgsIndirectBuffer, offset);
			}
		}
	}

} // namespace MG


