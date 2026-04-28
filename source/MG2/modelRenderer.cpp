// =======================================================
// ModelRenderer
// ・MG2の描画パイプラインの中核
// ・GPU駆動（Compute Shader）でインスタンス生成・カリング
// ・Deferred Rendering + Shadow対応
// =======================================================

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

#include <vector>

#if _DEBUG
#include "input.h"
#include "spriteRenderer2D.h"
#endif

namespace MG {

	void ModelRenderer::StaticInit()
	{
		// =======================================================
		// 一元初期化
		// ・GBuffer用テクスチャ（Color / Normal / WorldPosition）生成
		// ・シャドウマップ用テクスチャ生成
		// ・深度テクスチャ生成
		// =======================================================

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
			DIRECTIONAL_SHADOW_TEXTURE_WIDTH, DIRECTIONAL_SHADOW_TEXTURE_HEIGHT,
			DXGI_FORMAT_R32_TYPELESS, 
			D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE
		);
		s_DirectionalShadowSRV = Renderer::CreateTextureSRV(s_DirectionalShadowTexture, DXGI_FORMAT_R32_FLOAT);
		s_DirectionalShadowDSV = Renderer::CreateTextureDSV(s_DirectionalShadowTexture, DXGI_FORMAT_D32_FLOAT);
		
		s_DepthTexture = Renderer::CreateTexture2D(
			MGUtility::GetScreenWidth(), MGUtility::GetScreenHeight(),
			DXGI_FORMAT_R24G8_TYPELESS, D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE
		);
		s_DSV = Renderer::CreateTextureDSV(s_DepthTexture, DXGI_FORMAT_D24_UNORM_S8_UINT);
		s_DepthSRV = Renderer::CreateTextureSRV(s_DepthTexture, DXGI_FORMAT_R24_UNORM_X8_TYPELESS);
	}

	void ModelRenderer::StaticUninit()
	{

		SAFE_RELEASE(s_MeshInstanceUAV);
		SAFE_RELEASE(s_MeshInstanceSRV);
		SAFE_RELEASE(s_MeshInstanceBuffer);

		SAFE_RELEASE(s_MeshInstanceIndexUAV);
		SAFE_RELEASE(s_MeshInstanceIndexSRV);
		SAFE_RELEASE(s_MeshInstanceIndexBuffer);

		SAFE_RELEASE(s_ColorSRV);
		SAFE_RELEASE(s_ColorRTV);
		SAFE_RELEASE(s_ColorTexture);

		SAFE_RELEASE(s_NormalSRV);
		SAFE_RELEASE(s_NormalRTV);
		SAFE_RELEASE(s_NormalTexture);

		SAFE_RELEASE(s_WorldPositionSRV);
		SAFE_RELEASE(s_WorldPositionRTV);
		SAFE_RELEASE(s_WorldPositionTexture);

		SAFE_RELEASE(s_DirectionalShadowDSV);
		SAFE_RELEASE(s_DirectionalShadowSRV);
		SAFE_RELEASE(s_DirectionalShadowTexture);

		SAFE_RELEASE(s_DepthSRV);
		SAFE_RELEASE(s_DSV);
		SAFE_RELEASE(s_DepthTexture);

		s_MeshInstanceBufferCapacity = 0;
		s_MeshInstanceMax = 0;

		for (auto& pair : s_SceneModelSet) {
			for (auto& modelSetPair : pair.second) {
				SAFE_RELEASE(modelSetPair.second.modelInstanceIdSRV);
				SAFE_RELEASE(modelSetPair.second.modelInstanceIdBuffer);
				for (ModelInstance& modelInstance : modelSetPair.second.modelInstances) {
					modelInstance.Release();
				}
			}
		}
		s_SceneModelSet.clear();
	}

	void ModelRenderer::UpdateAll(Scene* scene, std::vector<ModelRenderer*>& components)
	{
		// =======================================================
		// 全モデル更新（1フレームのGPU前処理）
		// ・Transform更新
		// ・アニメーション（CS）
		// ・ボーン追従（CS）
		// ・MeshInstance展開（CS）
		// ・DrawArgs構築
		// =======================================================

		scene->UpdateGameObjectWorlds();
		DynamicMatrix::Update();
		Material::Update();
		ModelInstance::Update();
		DynamicIndexDivision::Update();
		Mesh::Update();
		ModelAnimation::Update();
		AnimationSet::Update();
		AnimationFollower::Update();

		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();

		// 時刻定数更新
		TIME_CONSTANT timeConstant{};
		timeConstant.currentTime = MGUtility::GetRunTimeMilliseconds();
		Renderer::SetTimeContant(timeConstant);

		// =======================================================
		// アニメーション計算
		// ・ノード行列をGPUで生成
		// ・インスタンス数 × ノード数でDispatch
		// =======================================================
		{
			// シェーダ
			static ID3D11ComputeShader* animationModelCS = Renderer::GetComputeShader("CS/animationCS.cso");
			deviceContext->CSSetShader(animationModelCS, NULL, 0);

			// UAV
			ID3D11UnorderedAccessView* uavArray[] = {
				MatrixDivision::GetDataUAV()
			};
			deviceContext->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavArray), uavArray, nullptr);

			// SRV
			ID3D11ShaderResourceView* srvArray[] = {
				MatrixDivision::GetBookmarkSRV(),
				TransformDivision::GetBookmarkSRV(),
				DynamicIndexDivision::GetBookmarkSRV(),
				ModelInstance::GetSRV(),
				AnimationSet::GetSRV(),
				ModelAnimation::GetSRV(),
				TransformDivision::GetDataSRV(),
				DynamicIndexDivision::GetDataSRV()
			};
			deviceContext->CSSetShaderResources(1, ARRAYSIZE(srvArray), srvArray);

			for (auto& pair : s_SceneModelSet[scene]) {
				Model model = pair.first;
				auto& modelData = model.GetData();
				MODEL_SET& modelSet = pair.second;
				auto& modelInstances = modelSet.modelInstances;
				unsigned int activeInstanceCount = static_cast<unsigned int>(modelSet.modelInstances.size() - modelSet.emptyIds.size());

				if (modelData.hasAnimation && activeInstanceCount > 0) {

					// モデルインスタンスID
					deviceContext->CSSetShaderResources(0, 1, &modelSet.modelInstanceIdSRV);

					// 上限定数
					CS_CONSTANT constant{};
					constant.CSMaxX = modelData.nodeCount;
					Renderer::SetCSContant(constant);

					// 実行
					deviceContext->Dispatch(static_cast<UINT>(ceil((float)modelData.nodeCount / 64)), modelInstances.size(), 1);
				}
			}
		}

		// =======================================================
		// アニメーション追従
		// ・指定ノードに対してオブジェクトを追従させる
		// ・武器・エフェクトなど
		// =======================================================
		{
			// シェーダ
			static ID3D11ComputeShader* animationFollowCS = Renderer::GetComputeShader("CS/animationFollowCS.cso");
			deviceContext->CSSetShader(animationFollowCS, NULL, 0);

			// UAV
			ID3D11UnorderedAccessView* dynamicMatrixUAV = DynamicMatrix::GetUAV();
			deviceContext->CSSetUnorderedAccessViews(0, 1, &dynamicMatrixUAV, nullptr);

			// SRV
			ID3D11ShaderResourceView* srvArray[] = {
				AnimationFollower::GetSRV(),
				ModelInstance::GetSRV(),
				MatrixDivision::GetBookmarkSRV(),
				MatrixDivision::GetDataSRV()
			};
			deviceContext->CSSetShaderResources(0, ARRAYSIZE(srvArray), srvArray);

			// 上限定数
			CS_CONSTANT constant{};
			constant.CSMaxX = static_cast<unsigned int>(AnimationFollower::GetCount());
			Renderer::SetCSContant(constant);

			// 実行
			deviceContext->Dispatch(static_cast<UINT>(ceil((float)AnimationFollower::GetCount() / 64)), 1, 1);
		}

		// =======================================================
		// MeshInstance生成
		// ・モデルインスタンス → メッシュ単位へ展開
		// ・AABB計算もここで実施
		// ・DrawArgs.instanceMaxCountを更新
		// =======================================================

		// Meshインスタンスのカウント
		unsigned int maxMeshInstanceCount = 0;

		for (auto& pair : s_SceneModelSet[scene]) {
			Model model = pair.first;
			auto& modelData = model.GetData();
			MODEL_SET& modelSet = pair.second;
			auto& modelInstances = modelSet.modelInstances;

			unsigned int activeInstanceCount = static_cast<unsigned int>(modelSet.modelInstances.size() - modelSet.emptyIds.size());

			if (activeInstanceCount == 0) continue;

			// バッファ確保
			unsigned int newCapacity = static_cast<unsigned int>(modelInstances.capacity());
			if (newCapacity > modelSet.bufferCapacity) {
				SAFE_RELEASE(modelSet.modelInstanceIdSRV);
				SAFE_RELEASE(modelSet.modelInstanceIdBuffer);

				modelSet.modelInstanceIdBuffer = Renderer::CreateStructuredBuffer(
					sizeof(ModelInstance), newCapacity, modelInstances.data()
				);
				if (modelSet.modelInstanceIdBuffer) {
					modelSet.modelInstanceIdSRV = Renderer::CreateStructuredSRV(modelSet.modelInstanceIdBuffer, newCapacity);
					modelSet.bufferCapacity = newCapacity;
					modelSet.needUpdateModelInstanceBuffer = false;
				}
			}

			// このモデルに属するインスタンス番号をバッファに更新
			if (modelSet.needUpdateModelInstanceBuffer && modelSet.modelInstanceIdBuffer) {
				D3D11_BOX box = Renderer::GetRangeBox(0, static_cast<unsigned int>(sizeof(ModelInstance) * modelInstances.size()));
				deviceContext->UpdateSubresource(modelSet.modelInstanceIdBuffer, 0, &box, modelInstances.data(), 0, 0);
				modelSet.needUpdateModelInstanceBuffer = false;
			}

			maxMeshInstanceCount += static_cast<unsigned int>(modelData.meshes.size()) * activeInstanceCount;
		}


		// Meshインスタンスのバッファを確保
		if (maxMeshInstanceCount > s_MeshInstanceBufferCapacity) {
			SAFE_RELEASE(s_MeshInstanceUAV);
			SAFE_RELEASE(s_MeshInstanceSRV);
			SAFE_RELEASE(s_MeshInstanceBuffer);

			SAFE_RELEASE(s_MeshInstanceIndexUAV);
			SAFE_RELEASE(s_MeshInstanceIndexSRV);
			SAFE_RELEASE(s_MeshInstanceIndexBuffer);

			// ちょっと多めに取っておく
			unsigned int newCapacity = static_cast<unsigned int>(maxMeshInstanceCount * 1.2);

			s_MeshInstanceBuffer = Renderer::CreateStructuredBuffer(
				sizeof(MESH_INSTANCE), newCapacity, nullptr,
				D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS
			);
			s_MeshInstanceSRV = Renderer::CreateStructuredSRV(s_MeshInstanceBuffer, newCapacity);
			s_MeshInstanceUAV = Renderer::CreateStructuredUAV(s_MeshInstanceBuffer, newCapacity, D3D11_BUFFER_UAV_FLAG_APPEND);

			s_MeshInstanceIndexBuffer = Renderer::CreateStructuredBuffer(
				sizeof(unsigned int), newCapacity, nullptr,
				D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS
			);
			s_MeshInstanceIndexSRV = Renderer::CreateStructuredSRV(s_MeshInstanceIndexBuffer, newCapacity);
			s_MeshInstanceIndexUAV = Renderer::CreateStructuredUAV(s_MeshInstanceIndexBuffer, newCapacity, D3D11_BUFFER_UAV_FLAG_APPEND);

			s_MeshInstanceBufferCapacity = newCapacity;
		}
		s_MeshInstanceMax = maxMeshInstanceCount;

		// Meshインスタンスを作成し、DrawArgにカウントを記録する
		{
			// シェーダ
			static ID3D11ComputeShader* expandMeshInstanceCS = Renderer::GetComputeShader("CS/expandMeshInstanceCS.cso");
			deviceContext->CSSetShader(expandMeshInstanceCS, NULL, 0);

			// UAV
			unsigned int counter = 0; // AppendBufferをリセット
			ID3D11UnorderedAccessView* drawArgsUav = Mesh::GetDrawArgsUAV();
			deviceContext->CSSetUnorderedAccessViews(0, 1, &drawArgsUav, nullptr);
			deviceContext->CSSetUnorderedAccessViews(1, 1, &s_MeshInstanceUAV, &counter);

			// SRV
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
				unsigned int activeInstanceCount = static_cast<unsigned int>(modelInstances.size() - modelSet.emptyIds.size());

				if (activeInstanceCount == 0) continue;

				// モデル定数
				MODEL_CONSTANT modelConstant{};
				modelConstant.modelId = model;
				modelConstant.nodeCount = modelData.nodeCount;
				modelConstant.maxInstance = static_cast<unsigned int>(modelInstances.size());
				modelConstant.nodeMatrixDivisionId = modelData.originalNodeMatrixDivision;
				Renderer::SetModelContant(modelConstant);

				// モデルインスタンスIDバッファ
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

					// 実行
					deviceContext->Dispatch(static_cast<UINT>(ceil((float)modelInstances.size() / 64)), 1, 1);
				}
			}
		}

		// =======================================================
		// DrawArgsのstartInstanceLocation計算
		// ・メッシュごとのinstance配列位置を確定
		// =======================================================
		{
			// シェーダ
			static ID3D11ComputeShader* offsetMeshInstanceCS = Renderer::GetComputeShader("CS/offsetMeshInstanceCS.cso");
			deviceContext->CSSetShader(offsetMeshInstanceCS, NULL, 0);

			// 上限定数
			CS_CONSTANT constant{};
			constant.CSMaxX = Mesh::GetDrawArgsCount();
			Renderer::SetCSContant(constant);

			// 実行
			deviceContext->Dispatch(1, 1, 1);
		}

	}

	void ModelRenderer::Culling()
	{
		// =======================================================
		// カリング処理
		// ・instanceCountリセット
		// ・Frustum + LODカリング
		// ・描画対象Indexを生成
		// ・DrawIndirect用Argsにコピー
		// =======================================================

		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();

		// 描画するMeshインスタンスのカウンターをリセットする
		{
			// シェーダ
			static ID3D11ComputeShader* resetInstanceCountCS = Renderer::GetComputeShader("CS/resetInstanceCountCS.cso");
			deviceContext->CSSetShader(resetInstanceCountCS, NULL, 0);

			// UAV
			ID3D11UnorderedAccessView* uavArray[] = {
				Mesh::GetDrawArgsUAV()
			};
			deviceContext->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavArray), uavArray, nullptr);

			// 上限定数
			CS_CONSTANT constant{};
			constant.CSMaxX = Mesh::GetDrawArgsCount();
			Renderer::SetCSContant(constant);

			// 実行
			deviceContext->Dispatch(static_cast<UINT>(ceil((float)constant.CSMaxX / 64)), 1, 1);
		}

		// 描画するMeshインスタンスIDを選別する
		{
			// シェーダ
			static ID3D11ComputeShader* frustumCullingCS = Renderer::GetComputeShader("CS/frustumCullingCS.cso");
			deviceContext->CSSetShader(frustumCullingCS, NULL, 0);

			// UAV
			ID3D11UnorderedAccessView* uavArray[] = {
				Mesh::GetDrawArgsUAV(),
				s_MeshInstanceUAV,
				s_MeshInstanceIndexUAV
			};
			deviceContext->CSSetUnorderedAccessViews(0, ARRAYSIZE(uavArray), uavArray, nullptr);

			// SRV
			ID3D11ShaderResourceView* srvArray[] = {
				ModelInstance::GetSRV(),
				DynamicMatrix::GetSRV()
			};
			deviceContext->CSSetShaderResources(0, ARRAYSIZE(srvArray), srvArray);

			// 上限定数
			CS_CONSTANT constant{};
			constant.CSMaxX = s_MeshInstanceMax;
			Renderer::SetCSContant(constant);

			// 実行
			deviceContext->Dispatch(static_cast<UINT>(ceil((float)s_MeshInstanceMax / 64)), 1, 1);

			// UAV解除
			ID3D11UnorderedAccessView* nullUAVs[] = {
				nullptr,
				nullptr,
				nullptr
			};
			deviceContext->CSSetUnorderedAccessViews(0, ARRAYSIZE(nullUAVs), nullUAVs, nullptr);

			// 選別結果をindirectDrawのArgsにコピーする
			ID3D11Buffer* drawArgsBuffer = Mesh::GetDrawArgsBuffer();
			ID3D11Buffer* indirectArgsBuffer = Mesh::GetDrawArgsIndirectBuffer();
			deviceContext->CopyResource(indirectArgsBuffer, drawArgsBuffer);

		}
		
	}

	void ModelRenderer::DrawAll(Scene* scene)
	{
		// =======================================================
		// 描画処理（DrawIndirect）
		// ・Mesh単位でループ
		// ・GPUが決めたinstanceCountで描画
		// =======================================================

		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();

		// 頂点バッファ
		UINT strides[] = {
			sizeof(VERTEX)
		};
		UINT offsets[] = { 0 };
		ID3D11Buffer* vertexBuffers[] = {
			VertexDivision::GetDataBuffer()
		};
		deviceContext->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);

		// 頂点インデックスバッファ
		deviceContext->IASetIndexBuffer(VertexIndexDivision::GetDataBuffer(), DXGI_FORMAT_R32_UINT, 0);

		// VS SRV
		ID3D11ShaderResourceView* srvArray[] = {
			s_MeshInstanceSRV,
			ModelInstance::GetSRV(),
			MatrixDivision::GetBookmarkSRV(),
			MatrixDivision::GetDataSRV(),
			s_MeshInstanceIndexSRV,
			DynamicMatrix::GetSRV(),
			Mesh::GetDrawArgsSRV(),

			// スキニング
			BoneDivision::GetDataSRV(),
			VertexBoneWeightDivision::GetDataSRV()
			
		};
		deviceContext->VSSetShaderResources(0, ARRAYSIZE(srvArray), srvArray);

		// PS SRV
		ID3D11ShaderResourceView* MaterialSRV = Material::GetSRV();
		deviceContext->PSSetShaderResources(3, 1, &MaterialSRV);

		// InDirect Args
		ID3D11Buffer* indirectArgsBuffer = Mesh::GetDrawArgsIndirectBuffer();

		for (auto& pair : s_SceneModelSet[scene]) {
			Model model = pair.first;
			auto& modelData = model.GetData();
			MODEL_SET& modelSet = pair.second;
			auto& modelInstances = modelSet.modelInstances;
			unsigned int activeInstanceCount = static_cast<unsigned int>(modelSet.modelInstances.size() - modelSet.emptyIds.size());

			if (activeInstanceCount == 0) continue;
			
			auto& nodeMeshPairs = modelData.nodeMeshPairs;
			for (int i = static_cast<int>(nodeMeshPairs.size()) - 1; i >= 0; i--) {
				auto& pair = nodeMeshPairs[i];
				Mesh mesh = modelData.meshes[pair.meshOffset];

				auto& meshData = mesh.GetData();

				// メッシュ定数
				MESH_CONSTANT meshConstant{};
				meshConstant.meshId = mesh;
				meshConstant.nodeIndex = pair.nodeOffset;
				if (meshData.boneDivision) {
					meshConstant.skinning = true;
					meshConstant.boneDivisionOffset = meshData.boneDivision.GetBookmarkData().offset;
					meshConstant.vertexBoneWeightDivisionOffset = meshData.vertexBoneWeightDivision.GetBookmarkData().offset;
				}
				Renderer::SetMeshContant(meshConstant);

				// テクスチャ
				Material material = modelData.materials[meshData.materialOffset];
				ID3D11ShaderResourceView* textureSRVs[] = {
					material.GetData().baseTexture.GetSRV(),
					material.GetData().normalTexture.GetSRV(),
					material.GetData().opacityTexture.GetSRV()
				};
				deviceContext->PSSetShaderResources(0, ARRAYSIZE(textureSRVs), textureSRVs);

				// デプスステート
				if (material.GetData().opaque) {
					Renderer::SetDepthState(DEPTH_STATE_COMPARISON_LESS);
				}
				else {
					Renderer::SetDepthState(DEPTH_STATE_NO_WRITE_COMPARISON_LESS);
				}

				// トポロジー
				Renderer::SetPrimitiveTopology(meshData.primitiveType);
				
				// ドロー
				unsigned int offset = sizeof(DRAW_INDEXED_INDIRECT_ARGS) * static_cast<unsigned int>(mesh);
				deviceContext->DrawIndexedInstancedIndirect(indirectArgsBuffer, offset);
			}
		}
	}

	void ModelRenderer::MainDrawAll(Scene* scene, std::vector<ModelRenderer*>& components)
	{
		// =======================================================
		// メイン描画
		// 1. シャドウマップ生成
		// 2. GBuffer生成
		// 3. Deferred Lighting
		// =======================================================

		ID3D11DeviceContext* deviceContext = Renderer::GetDeviceContext();
		
		static XMFLOAT4X4 directionalShadowViewProjection;

		// =======================================================
		// シャドウマップ描画
		// ・ライト視点で描画
		// ・深度のみ
		// =======================================================
		{
			// シャドーマップ用のカメラ定数を設定
			{
				Camera* camera = scene->GetMainCamera();

				XMVECTOR forward = scene->GetDirectLightDirection();
				XMVECTOR position = 
					camera->GetPosition() + 
					XMVector3Normalize(camera->GetForward() * Vector3(1.0f, 0.0f, 1.0f)) * DIRECTIONAL_SHADOW_PROJECTION_RANGE + 
					forward * DIRECTIONAL_SHADOW_PROJECTION_DISTANCE * -0.5f;

				XMMATRIX view = XMMatrixLookAtLH(position, position + forward, {0.0f, 0.0f, 1.0f});

				XMMATRIX projection = XMMatrixOrthographicOffCenterLH(
					-DIRECTIONAL_SHADOW_PROJECTION_RANGE, DIRECTIONAL_SHADOW_PROJECTION_RANGE,
					-DIRECTIONAL_SHADOW_PROJECTION_RANGE, DIRECTIONAL_SHADOW_PROJECTION_RANGE, 
					0.0f, DIRECTIONAL_SHADOW_PROJECTION_DISTANCE
				);

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

			// デプスバッファをクリアする
			deviceContext->ClearDepthStencilView(s_DirectionalShadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

			// シェーダ
			static auto vertexShaderSet = Renderer::GetVertexShaderSet("VS/modelInstancedVS.cso");
			static ID3D11InputLayout* inputLayout = vertexShaderSet.inputLayouts["general.csv"];
			deviceContext->VSSetShader(vertexShaderSet.vertexShader, NULL, 0);
			deviceContext->IASetInputLayout(inputLayout);
			deviceContext->PSSetShader(nullptr, NULL, 0);

			// レンダーターゲット
			ID3D11RenderTargetView* rtvArray[] = {
				nullptr,
				nullptr,
				nullptr
			};
			deviceContext->OMSetRenderTargets(ARRAYSIZE(rtvArray), rtvArray, s_DirectionalShadowDSV);

			// ビューポート
			Renderer::SetViewport(DIRECTIONAL_SHADOW_TEXTURE_WIDTH, DIRECTIONAL_SHADOW_TEXTURE_HEIGHT);

			// 描画
			DrawAll(scene);
		}

		// =======================================================
		// GBuffer描画
		// ・Color / Normal / WorldPosition出力
		// ・アウトライン（GeometryShader）
		// =======================================================
		{
			// レンダーターゲットをクリア
			static const float CLEAR_COLOR[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			deviceContext->ClearRenderTargetView(s_ColorRTV, CLEAR_COLOR);
			deviceContext->ClearRenderTargetView(s_NormalRTV, CLEAR_COLOR);
			deviceContext->ClearRenderTargetView(s_WorldPositionRTV, CLEAR_COLOR);
			deviceContext->ClearDepthStencilView(s_DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

			// カメラ設定
			scene->GetMainCamera()->Apply();

			Culling();

			// シェーダ
			static auto vertexShaderSet = Renderer::GetVertexShaderSet("VS/modelInstancedVS.cso");
			static ID3D11InputLayout* inputLayout = vertexShaderSet.inputLayouts["general.csv"];
			static ID3D11PixelShader* pixelShader = Renderer::GetPixelShader("PS/unlitTexturePS.cso");
			static ID3D11GeometryShader* outlineGS = Renderer::GetGeometryShader("GS/outlineGS.cso");
			deviceContext->VSSetShader(vertexShaderSet.vertexShader, NULL, 0);
			deviceContext->IASetInputLayout(inputLayout);
			deviceContext->PSSetShader(pixelShader, NULL, 0);
			deviceContext->GSSetShader(outlineGS, NULL, 0); // アウトライン、ToDo: フラグ化

			// レンダーターゲット
			ID3D11RenderTargetView* rtvArray[] = {
				s_ColorRTV,
				s_NormalRTV,
				s_WorldPositionRTV
			};
			deviceContext->OMSetRenderTargets(ARRAYSIZE(rtvArray), rtvArray, Renderer::GetMainDepthStencilView());

			// ビューポート
			Renderer::SetViewport(static_cast<float>(MGUtility::GetScreenWidth()), static_cast<float>(MGUtility::GetScreenHeight()));

			// 描画
			DrawAll(scene);

			// アウトラインoff
			deviceContext->GSSetShader(nullptr, NULL, 0);

		}

		// =======================================================
		// デファードライティング
		// ・シャドウ適用
		// =======================================================
		{
			// シェーダ
			static ID3D11VertexShader* vertexShader = Renderer::GetVertexShaderSet("VS/fullScreenVS.cso").vertexShader;
			static ID3D11PixelShader* pixelShader = Renderer::GetPixelShader("PS/deferredLightPS.cso");
			deviceContext->VSSetShader(vertexShader, NULL, 0);
			deviceContext->IASetInputLayout(nullptr);
			deviceContext->PSSetShader(pixelShader, NULL, 0);

			// レンダーターゲット
			Renderer::SetMainRenderTarget();

			// SRV
			ID3D11ShaderResourceView* srvArray[] = {
			s_ColorSRV,
			s_NormalSRV,
			s_WorldPositionSRV,
			s_DirectionalShadowSRV,
			s_DepthSRV
			};
			deviceContext->PSSetShaderResources(0, ARRAYSIZE(srvArray), srvArray);

			// ライト定数
			LIGHT_CONSTANT lightConstant{};
			lightConstant.ambient = Vector4(scene->GetAmbient());
			lightConstant.directLightColor = Vector4(scene->GetDirectLightColor());
			lightConstant.directLightDirection = Vector4(scene->GetDirectLightDirection());
			lightConstant.directionalShadowViewProjection = directionalShadowViewProjection;
			Renderer::SetLight(lightConstant);

			Renderer::SetDepthState(DEPTH_STATE_NO_WRITE_COMPARISON_ALWAYS);
			deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

			deviceContext->Draw(4, 0);
		}
		
	}

	void ModelRenderer::SetModel(Model model, unsigned int lod) 
	{
		// =======================================================
		// モデル設定
		// ・GameObjectに対応するモデルインスタンスを登録
		// ・インスタンスプールを利用して再利用（new/delete削減）
		// ・GPU転送用バッファ更新フラグも管理
		// =======================================================

		// 所属GameObject取得（存在しなければ何もしない）
		GameObject* gameObject = GetGameObject();
		if (!gameObject) return;

		// 所属Scene取得（存在しなければ何もしない）
		Scene* scene = gameObject->GetScene();
		if (!scene) return;

		// 同じモデルなら何もしない（無駄な更新防止）
		if (m_Model == model) return;

		// 既に登録済みのインスタンスがあれば無効化してプールへ戻す
		if (m_ModelSet) {
			m_ModelSet->modelInstances[m_InstanceIndex].SetEnabled(false);
			m_ModelSet->emptyIds.insert(m_InstanceIndex); // 再利用用
		}

		// モデルがnullなら解除のみ行って終了
		if (!model) {
			m_ModelSet = nullptr;
			return;
		}

		// 新しいモデルを設定
		m_Model = model;

		// Sceneごとの「モデル単位インスタンス管理」から取得
		MODEL_SET& modelSet = s_SceneModelSet[scene][model];
		m_ModelSet = &modelSet;

		// =======================================================
		// インスタンス確保（プール or 新規生成）
		// =======================================================

		if (modelSet.emptyIds.empty()) {
			// 空きがない → 新規生成
			modelSet.modelInstances.push_back(
				ModelInstance::Create(
					model,
					gameObject->GetWorldMatrix(),
					IsActive(),
					lod
				)
			);

			// 自分のインスタンスIndex
			m_InstanceIndex = static_cast<unsigned int>(modelSet.modelInstances.size() - 1);

			// GPU側バッファ更新が必要
			m_ModelSet->needUpdateModelInstanceBuffer = true;
		}
		else {
			// 空きIDを再利用
			m_InstanceIndex = *modelSet.emptyIds.begin();
			modelSet.emptyIds.erase(modelSet.emptyIds.begin());

			// インスタンス内容を更新
			modelSet.modelInstances[m_InstanceIndex].SetWorld(gameObject->GetWorldMatrix());
			modelSet.modelInstances[m_InstanceIndex].SetEnabled(IsActive());
			modelSet.modelInstances[m_InstanceIndex].SetLOD(lod);
		}
	}

	void ModelRenderer::SetAnimation(unsigned char animationId, unsigned int blendDuration, unsigned int timeOffset)
	{
		// =======================================================
		// アニメーション設定
		// ・現在アニメーション → 新しいアニメーションへ遷移
		// ・時間ベースでブレンド制御
		// =======================================================

		// モデル未設定なら何もしない
		if (!m_ModelSet) return;

		// 現在時刻（ミリ秒）
		unsigned int nowTime = MG::MGUtility::GetRunTimeMilliseconds();

		// 現在のAnimationSet取得
		AnimationSet animationSet =
			m_ModelSet->modelInstances[m_InstanceIndex].GetData().animationSet;

		AnimationSet::DATA animationSetData{};

		// =======================================================
		// 既存アニメーション → from側へ退避
		// =======================================================

		if (animationSet) {
			animationSetData = animationSet.GetData();

			// 現在のtoをfromへコピー（ブレンド開始のため）
			animationSetData.modelAnimationsFrom[0] =
				animationSetData.modelAnimationsTo[0];

			animationSetData.animationStartTimeFrom[0] =
				animationSetData.animationStartTimeTo[0];

			animationSetData.countFrom = animationSetData.countTo;
		}

		// =======================================================
		// 新しいアニメーションをto側へ設定
		// =======================================================

		animationSetData.modelAnimationsTo[0] =
			m_Model.GetData().animations[animationId];

		// 開始時間（timeOffsetで少し進めた状態にもできる）
		animationSetData.animationStartTimeTo[0] =
			nowTime - timeOffset;

		animationSetData.countTo = 1;

		// =======================================================
		// ブレンド設定
		// =======================================================

		animationSetData.animationBlendStartTime =
			nowTime - timeOffset;

		animationSetData.animationBlendDuration =
			blendDuration;

		// 再生速度（現状固定）
		animationSetData.timeMultiplier = 1.0f;

		// =======================================================
		// AnimationSetの適用
		// =======================================================

		if (!animationSet) {
			// 初回 → 新規生成
			animationSet = AnimationSet::Create(animationSetData);

			m_ModelSet->modelInstances[m_InstanceIndex]
				.SetAnimationSet(animationSet);
		}
		else {
			// 既存 → データ更新のみ
			animationSet.SetData(animationSetData);
		}

	}

	void ModelRenderer::OnAvtiveUpdated(bool newActive)
	{
		if (!m_ModelSet) return;
		m_ModelSet->modelInstances[m_InstanceIndex].SetEnabled(newActive);
	}

}; // namespace MG

