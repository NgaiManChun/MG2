#pragma once
#include "component.h"
#include "model.h"
#include "material.h"
#include "modelInstance.h"
#include "dynamicIndexDivision.h"
#include <unordered_map>
#include <vector>
#include <set>

struct ID3D11UnorderedAccessView;
struct ID3D11Texture2D;
struct ID3D11RenderTargetView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;

namespace MG {


	class ModelRenderer : public Component {
	private:
		struct MESH_INSTANCE
		{
			unsigned int meshId;
			unsigned int modelInstanceId;
			unsigned int nodeIndex;
			unsigned int materialId;
			Vector3 min;
			Vector3 max;
			unsigned int uz; // floatのz値をunsignedに直した値、透明オブジェクトのソート用
		};
		struct MODEL_SET{
			std::vector<ModelInstance> modelInstances;
			std::set<unsigned int> emptyIds;
			unsigned int bufferCapcity = 0;
			ID3D11Buffer* modelInstanceIdBuffer;
			ID3D11ShaderResourceView* modelInstanceIdSRV;
			bool needUpdateModelInstanceBuffer = false;
		};
		static inline std::unordered_map<Scene*, std::unordered_map<unsigned int, MODEL_SET>> s_SceneModelSet{};
		static const constexpr unsigned int INSTANCE_INTERVAL = 50;

		static inline ID3D11Buffer* s_MeshInstanceBuffer = nullptr;
		static inline ID3D11ShaderResourceView* s_MeshInstanceSRV = nullptr;
		static inline ID3D11UnorderedAccessView* s_MeshInstanceUAV = nullptr;
		static inline ID3D11Buffer* s_MeshInstanceIndexBuffer = nullptr;
		static inline ID3D11ShaderResourceView* s_MeshInstanceIndexSRV = nullptr;
		static inline ID3D11UnorderedAccessView* s_MeshInstanceIndexUAV = nullptr;
		static inline unsigned int s_MeshInstanceBufferCapcity = 0;
		static inline unsigned int s_MeshInstanceMax = 0;

		static inline ID3D11Buffer* s_FilteredIdBuffer;
		static inline ID3D11ShaderResourceView* s_FilteredIdSRV;
		static inline ID3D11UnorderedAccessView* s_FilteredIdUAV;
		
		static inline ID3D11Texture2D* s_ColorTexture = nullptr;
		static inline ID3D11RenderTargetView* s_ColorRTV = nullptr;
		static inline ID3D11ShaderResourceView* s_ColorSRV = nullptr;

		static inline ID3D11Texture2D* s_NormalTexture = nullptr;
		static inline ID3D11RenderTargetView* s_NormalRTV = nullptr;
		static inline ID3D11ShaderResourceView* s_NormalSRV = nullptr;

		static inline ID3D11Texture2D* s_WorldPositionTexture = nullptr;
		static inline ID3D11RenderTargetView* s_WorldPositionRTV = nullptr;
		static inline ID3D11ShaderResourceView* s_WorldPositionSRV = nullptr;

		static inline ID3D11Texture2D* s_DirectionalShadowTexture = nullptr;
		static inline ID3D11RenderTargetView* s_DirectionalShadowRTV = nullptr; // デバッグ用
		static inline ID3D11ShaderResourceView* s_DirectionalShadowSRV = nullptr;
		static inline ID3D11DepthStencilView* s_DirectionalShadowDSV = nullptr;

		static inline ID3D11Texture2D* s_DepthTexture = nullptr;
		static inline ID3D11DepthStencilView* s_DSV = nullptr;
		static inline ID3D11ShaderResourceView* s_DepthSRV = nullptr;
		
		static inline ID3D11Buffer* s_DebugBuffer = nullptr;
		

	public:
		static void StaticInit();
		static void StaticUninit();
		static void UpdateAll(Scene* scene);
		static void DrawAll(Scene* scene);
		static void MainDrawAll(Scene* scene);
		static void Culling();
	private:
		BIND_STATIC_INIT(ModelRenderer, ModelRenderer::StaticInit)
		BIND_STATIC_UNINIT(ModelRenderer, ModelRenderer::StaticUninit)
		BIND_UPDATE_ALL(ModelRenderer, ModelRenderer::UpdateAll)
		BIND_DRAW_ALL(ModelRenderer, ModelRenderer::MainDrawAll, 20)
		BIND_COMPONENT(ModelRenderer)

	private:
		Model m_Model;
		MODEL_SET* m_ModelSet = nullptr;
		unsigned int m_InstanceIndex = 0;

	public:
		ModelRenderer() {}

		void SetModel(Model model, unsigned int lod = LOD_ALL);

		Model GetModel() const { return m_Model; }

		ModelInstance GetModelInstance()
		{
			if (!m_ModelSet) return {};
			return m_ModelSet->modelInstances[m_InstanceIndex];
		}

		void SetMaterial(Material material, unsigned int index) {
			if (!m_ModelSet) return;
			m_ModelSet->modelInstances[m_InstanceIndex].SetMaterial(material, index);
		}

		void SetAnimation(unsigned char animationId, unsigned int blendDuration = 1000, unsigned int timeOffset = 0);

		AnimationSet GetAnimationSet() {
			if (!m_ModelSet) return {};
			return m_ModelSet->modelInstances[m_InstanceIndex].GetData().animationSet;
			
		}
	};
} // namespace MG

