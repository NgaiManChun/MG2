#pragma once
#include "renderer.h"
#include "component.h"
#include "vector3.h"
#include <array>

namespace MG {
	class SpotLight : public Component {
		struct SPOT_LIGHT{
			Matrix4x4 viewProjection;
			Vector3 position;
			Vector3 forward;
			float distance;
			Vector3 color;
			float blur;
			unsigned int enabled;
		};
	private:
		float m_Angle = XM_PIDIV2;
		float m_Near = 0.1f;
		float m_Far = 50.0f;
		Vector3 m_Color = { 1.0f, 1.0f, 1.0f };
		std::array<Vector3, 5> m_Frustum;
		SPOT_LIGHT m_Data;
		
	public:
		void Init() override;
		void SetAngle(float angle) { m_Angle = angle; }
		float GetAngle() const { return m_Angle; }
		void SetFar(float _far) { m_Far = _far; }
		float GetFar() const { return m_Far; }
		void SetColor(Vector3 color) { m_Color = color; }
		const Vector3& GetColor() const { return m_Color; }
		const std::array<Vector3, 5>& GetFrustum() { return m_Frustum; }
		void Apply();

		static void StaticInit();
		static void StaticUninit();
		static void UpdateAll(Scene* scene);
		static void DrawAll(Scene* scene);

	private:
		static inline ID3D11Texture2D* s_shadowTexArray = nullptr;
		static inline ID3D11ShaderResourceView* s_ArraySRV;
		static inline std::vector<ID3D11DepthStencilView*> s_DSVs;
		static inline unsigned int s_Capcity = 0;


		BIND_STATIC_INIT(SpotLight, SpotLight::StaticInit)
		BIND_STATIC_UNINIT(SpotLight, SpotLight::StaticUninit)
		BIND_UPDATE_ALL(SpotLight, SpotLight::UpdateAll)
		BIND_DRAW_ALL(SpotLight, SpotLight::DrawAll, 10)

		BIND_COMPONENT(SpotLight)
	};
} // namespace MG

