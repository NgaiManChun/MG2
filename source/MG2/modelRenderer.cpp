#include "modelRenderer.h"
#include "renderer.h"
#include "dynamicMatrix.h"
#include "texture.h"
#include "model.h"
#include "animation.h"
#include "modelAnimation.h"
#include "animationSet.h"
#include "vertexDivision.h"
#include "vertexIndexDivision.h"
#include "modelInstance.h"
#include "dynamicIndexDivision.h"
#include "animationFollower.h"
#include "camera.h"
#include "MGUtility.h"
#include "spriteRenderer2D.h"
#include <vector>

namespace MG {

	void ModelRenderer::SetModel(Model model, unsigned int lod) {
		GameObject* gameObject = GetGameObject();
		if (!gameObject) return;
		Scene* scene = gameObject->GetScene();
		if (!scene) return;

		if (m_Model == model) return;

		if (m_ModelSet) {
			m_ModelSet->modelInstances[m_InstanceIndex].SetEnabled(false);
			m_ModelSet->emptyIds.insert(m_InstanceIndex);
		}

		if (!model) {
			m_ModelSet = nullptr;
			return;
		}

		m_Model = model;
		MODEL_SET& modelSet = s_SceneModelSet[scene][model];
		m_ModelSet = &modelSet;
		
		if (modelSet.emptyIds.empty()) {
			modelSet.modelInstances.push_back(
				ModelInstance::Create(model, gameObject->GetWorldMatrix(), m_Enabled && gameObject->IsEnabled(), lod)
			);
			m_InstanceIndex = modelSet.modelInstances.size() - 1;
			m_ModelSet->needUpdateModelInstanceBuffer = true;
		}
		else {
			m_InstanceIndex = *modelSet.emptyIds.begin();
			modelSet.emptyIds.erase(modelSet.emptyIds.begin());
			modelSet.modelInstances[m_InstanceIndex].SetWorld(gameObject->GetWorldMatrix());
			modelSet.modelInstances[m_InstanceIndex].SetEnabled(m_Enabled && gameObject->IsEnabled());
			modelSet.modelInstances[m_InstanceIndex].SetLOD(lod);
		}
	}

	void ModelRenderer::SetAnimation(unsigned char animationId, unsigned int blendDuration, unsigned int timeOffset)
	{
		if (!m_ModelSet) return;
		unsigned int nowTime = MG::MGUtility::GetRunTimeMilliseconds();
		AnimationSet animationSet = m_ModelSet->modelInstances[m_InstanceIndex].GetData().animationSet;
		AnimationSet::DATA animationSetData{};
		

		if (animationSet) {
			animationSetData = animationSet.GetData();

			animationSetData.modelAnimationsFrom[0] = animationSetData.modelAnimationsTo[0];
			animationSetData.animationStartTimeFrom[0] = animationSetData.animationStartTimeTo[0];
			animationSetData.countFrom = animationSetData.countTo;
		}
		animationSetData.modelAnimationsTo[0] = m_Model.GetData().animations[animationId];
		animationSetData.animationStartTimeTo[0] = nowTime - timeOffset;
		animationSetData.countTo = 1;

		animationSetData.animationBlendStartTime = nowTime - timeOffset;
		animationSetData.animationBlendDuration = blendDuration;
		animationSetData.timeMultiplier = 1.0f;
		if (!animationSet) {
			animationSet = AnimationSet::Create(animationSetData);
			m_ModelSet->modelInstances[m_InstanceIndex].SetAnimationSet(animationSet);
		}
		else {
			animationSet.SetData(animationSetData);
		}
		
	}

	void ModelRenderer::StaticInit()
	{
		s_ColorTexture = Renderer::CreateTexture2D(MGUtility::GetScreenWidth(), MGUtility::GetScreenHeight());
		s_ColorRTV = Renderer::CreateTextureRTV(s_ColorTexture);
		s_ColorSRV = Renderer::CreateTextureSRV(s_ColorTexture);

		s_NormalTexture = Renderer::CreateTexture2D(MGUtility::GetScreenWidth(), MGUtility::GetScreenHeight());
		s_NormalRTV = Renderer::CreateTextureRTV(s_NormalTexture);
		s_NormalSRV = Renderer::CreateTextureSRV(s_NormalTexture);

		s_WorldPositionTexture = Renderer::CreateTexture2D(MGUtility::GetScreenWidth(), MGUtility::GetScreenHeight());
		s_WorldPositionRTV = Renderer::CreateTextureRTV(s_WorldPositionTexture);
		s_WorldPositionSRV = Renderer::CreateTextureSRV(s_WorldPositionTexture);

		s_DirectionalShadowTexture = Renderer::CreateTexture2D(
			400, 400,
			DXGI_FORMAT_R32_TYPELESS, 
			D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE
		);
		s_DirectionalShadowSRV = Renderer::CreateTextureSRV(s_DirectionalShadowTexture, DXGI_FORMAT_R32_FLOAT);
		s_DirectionalShadowDSV = Renderer::CreateTextureDSV(s_DirectionalShadowTexture, DXGI_FORMAT_D32_FLOAT);
		/*s_DirectionalShadowTexture = Renderer::CreateTexture2D(MGUtility::GetScreenWidth(), MGUtility::GetScreenHeight());
		s_DirectionalShadowSRV = Renderer::CreateTextureSRV(s_DirectionalShadowTexture);
		s_DirectionalShadowDSV = Renderer::CreateTextureDSV(s_DirectionalShadowTexture);
		s_DirectionalShadowRTV = Renderer::CreateTextureRTV(s_DirectionalShadowTexture);*/
		

		/*s_DepthTexture = Renderer::CreateTexture2D(
			MGUtility::GetScreenWidth(), MGUtility::GetScreenHeight(), 
			DXGI_FORMAT_D24_UNORM_S8_UINT, D3D11_BIND_DEPTH_STENCIL
		);*/
		s_DepthTexture = Renderer::CreateTexture2D(
			MGUtility::GetScreenWidth(), MGUtility::GetScreenHeight(),
			DXGI_FORMAT_R24G8_TYPELESS, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE
		);
		s_DSV = Renderer::CreateTextureDSV(s_DepthTexture, DXGI_FORMAT_D24_UNORM_S8_UINT);
		s_DepthSRV = Renderer::CreateTextureSRV(s_DepthTexture, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
	}

	void ModelRenderer::StaticUninit()
	{
		SAFE_RELEASE(s_ColorSRV);
		SAFE_RELEASE(s_ColorRTV);
		SAFE_RELEASE(s_NormalSRV);
		SAFE_RELEASE(s_NormalRTV);
		SAFE_RELEASE(s_WorldPositionSRV);
		SAFE_RELEASE(s_WorldPositionRTV);
		SAFE_RELEASE(s_DSV);
		SAFE_RELEASE(s_ColorTexture);
		SAFE_RELEASE(s_NormalTexture);
		SAFE_RELEASE(s_WorldPositionTexture);
		SAFE_RELEASE(s_DirectionalShadowRTV);
		SAFE_RELEASE(s_DirectionalShadowDSV);
		SAFE_RELEASE(s_DirectionalShadowSRV);
		SAFE_RELEASE(s_DirectionalShadowTexture);
		SAFE_RELEASE(s_DepthTexture);

	}

	void ModelRenderer::UpdateAll(Scene* scene)
	{

		scene->UpdateGameObjectWorlds();
		DynamicMatrix::Update();
		Material::Update();
		ModelInstance::Update();
		DynamicIndexDivision::Update();
		Mesh::Update();
		ModelAnimation::Update();
		AnimationSet::Update();
		AnimationFollower::Update();

		ID3D11Device* device = Renderer::GetDevice();
		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();

		TIME_CONSTANT timeConstant{};
		timeConstant.currentTime = MGUtility::GetRunTimeMilliseconds();
		Renderer::SetTimeContant(timeConstant);

		// アニメーション
		{
			static ID3D11ComputeShader* animationCS = Renderer::LoadComputeShader("complied_shader\\animationCS.cso");
			deviceContext->CSSetShader(animationCS, NULL, 0);

			CS_CONSTANT constant{};
			constant.CSMaxX = ModelInstance::GetCount();
			Renderer::SetCSContant(constant);

			ID3D11UnorderedAccessView* animationSetResultUAV = AnimationSet::GetResultUAV();
			deviceContext->CSSetUnorderedAccessViews(0, 1, &animationSetResultUAV, nullptr);

			ID3D11ShaderResourceView* srvArray[] = {
				ModelInstance::GetSRV(),
				AnimationSet::GetSRV(),
				ModelAnimation::GetSRV(),
				TransformDivision::GetBookmarkSRV(),
				DynamicIndexDivision::GetBookmarkSRV()
			};
			deviceContext->CSSetShaderResources(0, ARRAYSIZE(srvArray), srvArray);
			deviceContext->Dispatch(static_cast<UINT>(ceil((float)ModelInstance::GetCount() / 64)), 1, 1);


			

			
		}

		// AnimationFollower
		{

			static ID3D11ComputeShader* animationFollowCS = Renderer::LoadComputeShader("complied_shader\\animationFollowCS.cso");
			deviceContext->CSSetShader(animationFollowCS, NULL, 0);

			CS_CONSTANT constant{};
			constant.CSMaxX = AnimationFollower::GetCount();
			Renderer::SetCSContant(constant);

			ID3D11UnorderedAccessView* dynamicMatrixUAV = DynamicMatrix::GetUAV();
			deviceContext->CSSetUnorderedAccessViews(0, 1, &dynamicMatrixUAV, nullptr);

			ID3D11ShaderResourceView* srvArray[] = {
				AnimationFollower::GetSRV(),
				ModelInstance::GetSRV(),
				AnimationSet::GetResultSRV(),
				TransformDivision::GetDataSRV(),
				DynamicIndexDivision::GetDataSRV()
			};
			deviceContext->CSSetShaderResources(0, ARRAYSIZE(srvArray), srvArray);
			deviceContext->Dispatch(static_cast<UINT>(ceil((float)AnimationFollower::GetCount() / 64)), 1, 1);

			// DEBUG
			/*{
				ID3D11UnorderedAccessView* nullUAV = nullptr;
				deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);

				static ID3D11Buffer* debugBuffer = ([](unsigned int stride, unsigned int count) {
					D3D11_BUFFER_DESC desc = {};
					desc.Usage = D3D11_USAGE_STAGING;
					desc.BindFlags = 0;
					desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
					desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
					desc.StructureByteStride = stride;
					desc.ByteWidth = stride * count;
					ID3D11Buffer* debugBuffer;
					Renderer::GetDevice()->CreateBuffer(&desc, nullptr, &debugBuffer);
					return debugBuffer;
					})(sizeof(AnimationSet::RESULT), 20000);

				Renderer::GetDeviceContext()->CopySubresourceRegion(debugBuffer, 0, 0, 0, 0, debugResultBuffer, 0, nullptr);

				D3D11_MAPPED_SUBRESOURCE mapped;
				if (Renderer::GetDeviceContext()->Map(debugBuffer, 0, D3D11_MAP_READ, 0, &mapped) == S_OK) {
					AnimationSet::RESULT data[20000];
					memcpy(data, mapped.pData, sizeof(AnimationSet::RESULT) * 20000);
					Renderer::GetDeviceContext()->Unmap(debugBuffer, 0);
				}
			}*/
		}


		// Model Instance×Mesh のカウント
		unsigned int maxMeshInstanceCount = 0;

		for (auto& pair : s_SceneModelSet[scene]) {
			Model model = pair.first;
			auto& modelData = model.GetData();
			MODEL_SET& modelSet = pair.second;
			auto& modelInstances = modelSet.modelInstances;

			unsigned int activeInstanceCount = modelSet.modelInstances.size() - modelSet.emptyIds.size();

			if (activeInstanceCount == 0) continue;

			// このモデルに属するインスタンス番号をGPUに更新
			if (modelInstances.capacity() > modelSet.bufferCapcity) {
				SAFE_RELEASE(modelSet.modelInstanceIdSRV);
				SAFE_RELEASE(modelSet.modelInstanceIdBuffer);

				modelSet.modelInstanceIdBuffer = Renderer::CreateStructuredBuffer(
					sizeof(ModelInstance), modelInstances.capacity(), modelInstances.data()
				);
				if (modelSet.modelInstanceIdBuffer) {
					modelSet.modelInstanceIdSRV = Renderer::CreateStructuredSRV(modelSet.modelInstanceIdBuffer, modelInstances.capacity());
					modelSet.bufferCapcity = modelInstances.capacity();
					modelSet.needUpdateModelInstanceBuffer = false;
				}
			}
			if (modelSet.needUpdateModelInstanceBuffer) {
				D3D11_BOX box = Renderer::GetRangeBox(0, sizeof(ModelInstance) * modelInstances.size());
				deviceContext->UpdateSubresource(modelSet.modelInstanceIdBuffer, 0, &box, modelInstances.data(), 0, 0);
				modelSet.needUpdateModelInstanceBuffer = false;
			}

			maxMeshInstanceCount += static_cast<unsigned int>(modelData.meshes.size()) * activeInstanceCount;

		}


		// Model Instance×Meshを展開するためのバッファを準備
		if (maxMeshInstanceCount > s_MeshInstanceBufferCapcity) {
			SAFE_RELEASE(s_MeshInstanceUAV);
			SAFE_RELEASE(s_MeshInstanceSRV);
			SAFE_RELEASE(s_MeshInstanceBuffer);

			SAFE_RELEASE(s_MeshInstanceIndexUAV);
			SAFE_RELEASE(s_MeshInstanceIndexSRV);
			SAFE_RELEASE(s_MeshInstanceIndexBuffer);

			// ちょっと多めに取っておく
			unsigned int newCapcity = static_cast<unsigned int>(maxMeshInstanceCount * 1.2);

			s_MeshInstanceBuffer = Renderer::CreateStructuredBuffer(
				sizeof(MESH_INSTANCE), newCapcity, nullptr,
				D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS
			);
			s_MeshInstanceSRV = Renderer::CreateStructuredSRV(s_MeshInstanceBuffer, newCapcity);
			s_MeshInstanceUAV = Renderer::CreateStructuredUAV(s_MeshInstanceBuffer, newCapcity, D3D11_BUFFER_UAV_FLAG_APPEND);

			s_MeshInstanceIndexBuffer = Renderer::CreateStructuredBuffer(
				sizeof(unsigned int), newCapcity, nullptr,
				D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS
			);
			s_MeshInstanceIndexSRV = Renderer::CreateStructuredSRV(s_MeshInstanceIndexBuffer, newCapcity);
			s_MeshInstanceIndexUAV = Renderer::CreateStructuredUAV(s_MeshInstanceIndexBuffer, newCapcity, D3D11_BUFFER_UAV_FLAG_APPEND);

			s_MeshInstanceBufferCapcity = newCapcity;
		}
		s_MeshInstanceMax = maxMeshInstanceCount;
		

		// Model Instance×Meshを展開する
		{
			static ID3D11ComputeShader* expandMeshInstanceCS = Renderer::LoadComputeShader("complied_shader\\expandMeshInstanceCS.cso");
			deviceContext->CSSetShader(expandMeshInstanceCS, NULL, 0);

			unsigned int counter = 0; // AppendBufferをリセット
			ID3D11UnorderedAccessView* drawArgsUav = Mesh::GetDrawArgsUAV();
			deviceContext->CSSetUnorderedAccessViews(0, 1, &drawArgsUav, nullptr);
			deviceContext->CSSetUnorderedAccessViews(1, 1, &s_MeshInstanceUAV, &counter);

			ID3D11ShaderResourceView* srvArray[] = {
				ModelInstance::GetSRV(),
				DynamicIndexDivision::GetBookmarkSRV(),
				DynamicIndexDivision::GetDataSRV(),
				MatrixDivision::GetBookmarkSRV(),
				MatrixDivision::GetDataSRV(),
				DynamicMatrix::GetSRV()
			};
			deviceContext->CSSetShaderResources(1, ARRAYSIZE(srvArray), srvArray);

			for (auto& pair : s_SceneModelSet[scene]) {
				Model model = pair.first;
				auto& modelData = model.GetData();
				MODEL_SET& modelSet = pair.second;
				auto& modelInstances = modelSet.modelInstances;
				unsigned int activeInstanceCount = modelSet.modelInstances.size() - modelSet.emptyIds.size();

				if (activeInstanceCount == 0) continue;

				// モデル定数
				MODEL_CONSTANT modelConstant{};
				modelConstant.modelId = model;
				modelConstant.nodeCount = modelData.nodeCount;
				modelConstant.maxInstance = modelInstances.size();
				modelConstant.nodeMatrixDivisionId = modelData.originalNodeMatrixDivision;
				Renderer::SetModelContant(modelConstant);

				deviceContext->CSSetShaderResources(0, 1, &modelSet.modelInstanceIdSRV);

				auto& nodeMeshPairs = modelData.nodeMeshPairs;
				for (auto& pair : nodeMeshPairs) {
					Mesh mesh = modelData.meshes[pair.meshOffset];
					auto& meshData = mesh.GetData();

					// メッシュ定数
					MESH_CONSTANT meshConstant{};
					meshConstant.meshId = mesh;
					meshConstant.nodeIndex = pair.nodeOffset;
					meshConstant.meshMaterialOffset = meshData.materialOffset;
					meshConstant.localMin = meshData.min;
					meshConstant.localMax = meshData.max;
					Renderer::SetMeshContant(meshConstant);

					deviceContext->Dispatch(static_cast<UINT>(ceil((float)modelInstances.size() / 64)), 1, 1);

				}

			}
		}

		
		
		// 各DrawArgのinstanceStartLocationをセット
		static ID3D11ComputeShader* offsetMeshInstanceCS = Renderer::LoadComputeShader("complied_shader\\offsetMeshInstanceCS.cso");
		deviceContext->CSSetShader(offsetMeshInstanceCS, NULL, 0);
		CS_CONSTANT constant{};
		constant.CSMaxX = Mesh::GetDrawArgsCount();
		Renderer::SetCSContant(constant);
		deviceContext->Dispatch(1, 1, 1);

		
		

	}

	void ModelRenderer::Culling()
	{
		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();

		// カウンターをリセットする
		{
			ID3D11UnorderedAccessView* uavArray[] = {
				Mesh::GetDrawArgsUAV()
			};
			deviceContext->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavArray), uavArray, nullptr);

			static ID3D11ComputeShader* resetInstanceCountCS = Renderer::LoadComputeShader("complied_shader\\resetInstanceCountCS.cso");
			deviceContext->CSSetShader(resetInstanceCountCS, NULL, 0);

			CS_CONSTANT constant{};
			constant.CSMaxX = Mesh::GetDrawArgsCount();
			Renderer::SetCSContant(constant);
			deviceContext->Dispatch(static_cast<UINT>(ceil((float)constant.CSMaxX / 64)), 1, 1);
		}

		ID3D11UnorderedAccessView* uavArray[] = {
				Mesh::GetDrawArgsUAV(),
				s_MeshInstanceUAV,
				s_MeshInstanceIndexUAV
		};
		deviceContext->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavArray), uavArray, nullptr);
		static ID3D11ComputeShader* frustumCullingCS = Renderer::LoadComputeShader("complied_shader\\frustumCullingCS.cso");
		deviceContext->CSSetShader(frustumCullingCS, NULL, 0);

		CS_CONSTANT constant{};
		constant.CSMaxX = s_MeshInstanceMax;
		Renderer::SetCSContant(constant);

		ID3D11ShaderResourceView* srvArray[] = {
			ModelInstance::GetSRV(),
			DynamicMatrix::GetSRV()
		};
		deviceContext->CSSetShaderResources(0, ARRAYSIZE(srvArray), srvArray);
		deviceContext->Dispatch(static_cast<UINT>(ceil((float)s_MeshInstanceMax / 64)), 1, 1);

		ID3D11UnorderedAccessView* nullUAVs[] = {
			nullptr,
			nullptr,
			nullptr
		};
		deviceContext->CSSetUnorderedAccessViews(0, ARRAYSIZE(nullUAVs), nullUAVs, nullptr);

		ID3D11Buffer* drawArgsBuffer = Mesh::GetDrawArgsBuffer();
		ID3D11Buffer* indirectArgsBuffer = Mesh::GetDrawArgsIndirectBuffer();

		deviceContext->CopyResource(indirectArgsBuffer, drawArgsBuffer);

		
	}

	void ModelRenderer::DrawAll(Scene* scene)
	{
		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();

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

		ID3D11ShaderResourceView* srvArray[] = {
			s_MeshInstanceSRV,
			ModelInstance::GetSRV(),
			MatrixDivision::GetBookmarkSRV(),
			MatrixDivision::GetDataSRV(),
			s_MeshInstanceIndexSRV,
			DynamicMatrix::GetSRV(),
			Mesh::GetDrawArgsSRV(),

			// アニメーション
			AnimationSet::GetResultSRV(),
			TransformDivision::GetDataSRV(),
			DynamicIndexDivision::GetDataSRV(),

			// スキニング
			BoneDivision::GetDataSRV(),
			VertexBoneWeightDivision::GetDataSRV()
			
		};
		deviceContext->VSSetShaderResources(0, ARRAYSIZE(srvArray), srvArray);

		

		// InDirect Args
		ID3D11Buffer* indirectArgsBuffer = Mesh::GetDrawArgsIndirectBuffer();

		ID3D11ShaderResourceView* MaterialSRV = Material::GetSRV();
		deviceContext->PSSetShaderResources(3, 1, &MaterialSRV);

		
		// Renderer::SetBlendState(BLEND_STATE_ALPHA_A2C);
		for (auto& pair : s_SceneModelSet[scene]) {
			Model model = pair.first;
			auto& modelData = model.GetData();
			MODEL_SET& modelSet = pair.second;
			auto& modelInstances = modelSet.modelInstances;
			unsigned int activeInstanceCount = modelSet.modelInstances.size() - modelSet.emptyIds.size();

			if (activeInstanceCount == 0) continue;

			MODEL_CONSTANT modelConstant{};
			modelConstant.modelId = model;
			/*modelConstant.nodeCount = modelData.nodeCount;
			modelConstant.maxInstance = modelInstances.size();*/
			modelConstant.nodeMatrixDivisionId = modelData.originalNodeMatrixDivision;
			modelConstant.nodeParentIndexDivisionOffset = modelData.nodeParentIndexDivision.GetBookmarkData().offset;
			Renderer::SetModelContant(modelConstant);
			
			auto& nodeMeshPairs = modelData.nodeMeshPairs;
			for (int i = nodeMeshPairs.size() - 1; i >= 0; i--) {
				auto& pair = nodeMeshPairs[i];
				Mesh mesh = modelData.meshes[pair.meshOffset];

				auto& meshData = mesh.GetData();

				MESH_CONSTANT meshConstant{};
				meshConstant.meshId = mesh;
				meshConstant.nodeIndex = pair.nodeOffset;
				if (meshData.boneDivision) {
					meshConstant.skinning = true;
					//meshConstant.boneDivisionId = meshData.boneDivision;
					//meshConstant.vertexBoneWeightDivisionId = meshData.vertexBoneWeightDivision;
					meshConstant.boneDivisionOffset = meshData.boneDivision.GetBookmarkData().offset;
					meshConstant.vertexBoneWeightDivisionOffset = meshData.vertexBoneWeightDivision.GetBookmarkData().offset;
				}
				
				Renderer::SetMeshContant(meshConstant);

				Material material = modelData.materials[meshData.materialOffset];
				ID3D11ShaderResourceView* textureSRVs[] = {
					material.GetData().baseTexture.GetSRV(),
					material.GetData().normalTexture.GetSRV(),
					material.GetData().opacityTexture.GetSRV()
				};
				deviceContext->PSSetShaderResources(0, ARRAYSIZE(textureSRVs), textureSRVs);

				if (material.GetData().opaque) {
					Renderer::SetDepthState(DEPTH_STATE_COMPARISON_LESS);
				}
				else {
					Renderer::SetDepthState(DEPTH_STATE_NO_WRITE_COMPARISON_LESS);
				}

				Renderer::SetPrimitiveTopology(meshData.primitiveType);
				
				unsigned int offset = sizeof(DRAW_INDEXED_INDIRECT_ARGS) * static_cast<unsigned int>(mesh);
				deviceContext->DrawIndexedInstancedIndirect(indirectArgsBuffer, offset);
			}
		}
		//Renderer::SetBlendState(BLEND_STATE_ALPHA);
	}

	void ModelRenderer::MainDrawAll(Scene* scene)
	{
		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();
		static const float CLEAR_COLOR[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

		static XMFLOAT4X4 directionalShadowViewProjection;

		

		static bool cap = false;
		if (true) {

			{
				Camera* camera = scene->GetMainCamera();

				float x = 10.0f;

				XMVECTOR forward = scene->GetDirectLightDirection();
				XMVECTOR position = camera->GetPosition() + XMVector3Normalize(camera->GetForward() * Vector3(1.0f, 0.0f, 1.0f)) * x + forward * -50.0f;

				XMMATRIX view = XMMatrixLookAtLH(position, position + forward, {0.0f, 0.0f, 1.0f});

				
				XMMATRIX projection = XMMatrixOrthographicOffCenterLH(-x, x, -x, x, 0.0f, 100.0f);

				XMMATRIX invViewRotation = XMMatrixInverse(nullptr, view);
				invViewRotation.r[3].m128_f32[0] = 0.0f;
				invViewRotation.r[3].m128_f32[1] = 0.0f;
				invViewRotation.r[3].m128_f32[2] = 0.0f;

				XMMATRIX invVp = XMMatrixInverse(nullptr, view * projection);

				Vector3 points[4] = {
					{ -5.0f,  5.0f, 50.0f },
					{  5.0f,  5.0f, 50.0f },
					{ -5.0f, -5.0f, 50.0f },
					{  5.0f, -5.0f, 50.0f }
				};
				points[0] = XMVector3TransformCoord(points[0], invVp);
				points[1] = XMVector3TransformCoord(points[1], invVp);
				points[2] = XMVector3TransformCoord(points[2], invVp);
				points[3] = XMVector3TransformCoord(points[3], invVp);

				Vector4 frustum[] = {
					XMVector3Normalize(points[1] - points[0]),
					XMVector3Normalize(points[0] - points[1]),
					XMVector3Normalize(points[0] - points[2]),
					XMVector3Normalize(points[2] - points[0])
				};

				Vector4 frustumPoints[] = {
					points[0],
					points[1],
					points[2],
					points[0]
				};


				CAMERA_CONSTANT cameraConstant = {};
				XMStoreFloat4x4(&cameraConstant.view, XMMatrixTranspose(view));
				XMStoreFloat4x4(&cameraConstant.projection, XMMatrixTranspose(projection));
				XMStoreFloat4x4(&cameraConstant.viewProjection, XMMatrixTranspose(view * projection));
				XMStoreFloat4x4(&cameraConstant.invViewProjection, XMMatrixTranspose(XMMatrixInverse(nullptr, view * projection)));
				XMStoreFloat4x4(&cameraConstant.invViewRotation, XMMatrixTranspose(invViewRotation));
				XMStoreFloat3(&cameraConstant.cameraPosition, position);
				cameraConstant.projectionFar = 100.0f;
				memcpy(cameraConstant.frustum, frustum, sizeof(Vector4) * ARRAYSIZE(frustum));
				memcpy(cameraConstant.frustumPoints, frustumPoints, sizeof(Vector4) * ARRAYSIZE(frustumPoints));
				cameraConstant.forceLOD = LOD_4;
				XMStoreFloat3(&cameraConstant.cameraForward, forward);
				Renderer::SetCamera(cameraConstant);

				XMStoreFloat4x4(&directionalShadowViewProjection, XMMatrixTranspose(view * projection));
			}

			Culling();

			//deviceContext->ClearRenderTargetView(s_DirectionalShadowRTV, CLEAR_COLOR);
			deviceContext->ClearDepthStencilView(s_DirectionalShadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

			ID3D11RenderTargetView* rtvArray[] = {
				nullptr,
				nullptr,
				nullptr
			};
			deviceContext->OMSetRenderTargets(ARRAYSIZE(rtvArray), rtvArray, s_DirectionalShadowDSV);


			Renderer::SetViewport(400.0f, 400.0f);

			auto shaderSet = Renderer::GetShaderSet(SHADER_TYPE_UNLIT);
			deviceContext->VSSetShader(shaderSet.vertexShader, NULL, 0);
			deviceContext->IASetInputLayout(shaderSet.inputLayout);
			deviceContext->PSSetShader(nullptr, NULL, 0);

			Renderer::SetDepthState(DEPTH_STATE_COMPARISON_LESS);

			DrawAll(scene);
		}
		cap = !cap;

		

		
		{
			scene->GetMainCamera()->Apply();
			Culling();

			deviceContext->ClearRenderTargetView(s_ColorRTV, CLEAR_COLOR);
			deviceContext->ClearRenderTargetView(s_NormalRTV, CLEAR_COLOR);
			deviceContext->ClearRenderTargetView(s_WorldPositionRTV, CLEAR_COLOR);
			deviceContext->ClearDepthStencilView(s_DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

			ID3D11RenderTargetView* rtvArray[] = {
				s_ColorRTV,
				s_NormalRTV,
				s_WorldPositionRTV
			};
			deviceContext->OMSetRenderTargets(ARRAYSIZE(rtvArray), rtvArray, s_DSV);


			Renderer::SetViewport(static_cast<float>(MGUtility::GetScreenWidth()), static_cast<float>(MGUtility::GetScreenHeight()));

			auto shaderSet = Renderer::GetShaderSet(SHADER_TYPE_UNLIT);
			deviceContext->VSSetShader(shaderSet.vertexShader, NULL, 0);
			deviceContext->IASetInputLayout(shaderSet.inputLayout);
			deviceContext->PSSetShader(shaderSet.pixelShader, NULL, 0);

			Renderer::SetDepthState(DEPTH_STATE_COMPARISON_LESS);

			//
			static ID3D11GeometryShader* outlineGS = Renderer::LoadGeometryShader("complied_shader\\outlineGS.cso");
			deviceContext->GSSetShader(outlineGS, NULL, 0);
			DrawAll(scene);
			deviceContext->GSSetShader(nullptr, NULL, 0);
		}

		
		

		Renderer::SetMainRenderTarget();
		ID3D11ShaderResourceView* srvArray[] = {
			s_ColorSRV,
			s_NormalSRV,
			s_WorldPositionSRV,
			s_DirectionalShadowSRV,
			s_DepthSRV
		};
		deviceContext->PSSetShaderResources(0, ARRAYSIZE(srvArray), srvArray);

		auto shaderSet = Renderer::GetShaderSet(SHADER_TYPE_DEFERRED_LIGHT);
		deviceContext->VSSetShader(shaderSet.vertexShader, NULL, 0);
		deviceContext->IASetInputLayout(shaderSet.inputLayout);
		deviceContext->PSSetShader(shaderSet.pixelShader, NULL, 0);

		LIGHT_CONSTANT lightConstant{};
		lightConstant.ambient = Vector4(scene->GetAmbient());
		lightConstant.directLightColor = Vector4(scene->GetDirectLightColor());
		lightConstant.directLightDirection = Vector4(scene->GetDirectLightDirection());
		lightConstant.directionalShadowViewProjection = directionalShadowViewProjection;
		Renderer::SetLight(lightConstant);

		Renderer::SetDepthState(DEPTH_STATE_NO_WRITE_COMPARISON_ALWAYS);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		CS_CONSTANT constant{};
		constant.CSMaxZ = MGUtility::GetRunTimeMilliseconds();
		Renderer::SetCSContant(constant);
		deviceContext->Draw(4, 0);
	}

}; // namespace MG

