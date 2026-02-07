#pragma once

#include "renderer.h"
#include "component.h"
#include <array>

namespace MG {
	class Camera : public Component {
		BIND_COMPONENT_WITHOUT_DRAW(Camera)
	private:
		float m_Angle = 1.0f;
		float m_Near = 0.1f;
		float m_Far = 1000.0f;
		unsigned int m_ForceLOD;
		
	public:
		void Update() override;
		void SetAngle(float angle) { m_Angle = angle; }
		float GetAngle() const { return m_Angle; }
		void SetFar(float _far) { m_Far = _far; }
		void SetForceLOD(LOD lod) {
			m_ForceLOD = lod;
		}
		float GetFar() const { return m_Far; }
		void Apply();

		AABB GetProjectionAABB();
		
	};
} // namespace MG

