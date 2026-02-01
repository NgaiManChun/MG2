#pragma once
#include "component.h"
#include "model.h"
#include "material.h"
#include "modelInstance.h"
#include "dynamicIndexDivision.h"
#include <unordered_map>
#include <vector>
#include <set>

namespace MG {

	struct ID3D11Texture2D;
	struct ID3D11DepthStencilView;

	class DirectionalLight : public Component {
	private:


	public:
		static void DrawAll(Scene* scene);

	private:
		BIND_DRAW_ALL(DirectionalLight, DirectionalLight::DrawAll, 10)
		BIND_COMPONENT(DirectionalLight)

	private:
		Material m_Material;
		Texture m_Texture;
		ID3D11Texture2D* m_Texture2D;
		ID3D11ShaderResourceView* m_SRV;
		ID3D11DepthStencilView* m_DSV;

	public:
		void Init() override;
	};
} // namespace MG

