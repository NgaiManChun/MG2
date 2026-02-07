#pragma once
#include "component.h"
#include "model.h"
#include "material.h"
#include "modelInstance.h"
#include "dynamicIndexDivision.h"
#include "renderer.h"
#include "dataType.h"
#include "modelRenderer.h"
#include <unordered_map>
#include <vector>
#include <set>

struct ID3D11UnorderedAccessView;

namespace MG {


	class SkyRenderer : public Component {
	private:
		//inline static SHADER_SET s_ShaderSet;
	public:
		static void StaticInit();
		static void StaticUninit();
		static void UpdateAll(Scene* scene);
	private:
		/*BIND_STATIC_INIT(SkyRenderer, SkyRenderer::StaticInit)
		BIND_STATIC_UNINIT(SkyRenderer, SkyRenderer::StaticUninit)*/
		//BIND_UPDATE_ALL(SkyRenderer, SkyRenderer::UpdateAll)
		BIND_COMPONENT(SkyRenderer)

	private:
		Model m_Model;
	public:

		void Draw() override;
		void Update() override;

		void SetModel(Model model) 
		{
			m_Model = model;
		}
	};
} // namespace MG

