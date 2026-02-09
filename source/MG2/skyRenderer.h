// =======================================================
// skyRenderer.h
// 
// スカイスフィア描画用コンポーネント
// =======================================================
#pragma once
#include "component.h"
#include "model.h"

struct ID3D11UnorderedAccessView;

namespace MG {
	class SkyRenderer : public Component {
	private:
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

