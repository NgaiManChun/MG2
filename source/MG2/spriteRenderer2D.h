// =======================================================
// spriteRenderer2D.h
// 
// 2D四角sprite描画用コンポーネント
// =======================================================
#pragma once
#include "component.h"
#include "material.h"

namespace MG {

	class SpriteRenderer2D : public Component {
	public:
		static void DrawAll(Scene* scene);

	private:
		BIND_DRAW_ALL(SpriteRenderer2D, SpriteRenderer2D::DrawAll, 60)
		BIND_COMPONENT(SpriteRenderer2D)

	private:
		Material m_Material;

	public:
		SpriteRenderer2D() {}
		void SetMaterial(Material material) { m_Material = material; }

	};

} // namespace MG

