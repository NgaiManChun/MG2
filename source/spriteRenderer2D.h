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


	class SpriteRenderer2D : public Component {
	private:


	public:
		static void DrawAll(Scene* scene);

	private:
		BIND_DRAW_ALL(SpriteRenderer2D, SpriteRenderer2D::DrawAll, 60)
		BIND_COMPONENT(SpriteRenderer2D)

	private:
		Material m_Material;

	public:

		//void Draw() override;

		SpriteRenderer2D() {}

		void SetMaterial(Material material) {
			m_Material = material;
		}
	};
} // namespace MG

