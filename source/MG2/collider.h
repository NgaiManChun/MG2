#pragma once

#include "component.h"
#include "dataType.h"

namespace MG {

	class BoxCollider;

	class Collider : public Component {
		enum SHAPE {
			SHAPE_BOX
		};
		struct AABB {
			XMVECTOR min;
			XMVECTOR max;
			Collider* component;
			SHAPE shape;
		};
	private:
		static inline std::vector<AABB> s_AABBs;
	protected:
		size_t m_Tags;
		std::vector<Collider*> m_OverlapColliders;
		void AddAABB() {
			SHAPE shape;
			if (reinterpret_cast<BoxCollider*>(this)) {
				shape = SHAPE_BOX;
			}
			s_AABBs.push_back({
				{},
				{},
				this,
				shape
			});
		}
	public:
		static void UpdateAll(Scene* scene, std::vector<Collider*>& components);

		Collider(size_t tags = 0) : m_Tags(tags) {}
		void SetTags(size_t tags) { m_Tags = tags; }
		size_t GetTags() const { return m_Tags; }
		const std::vector<Collider*>& GetOverlapColliders() const { return m_OverlapColliders; }
		bool HasOverlap() const { return !m_OverlapColliders.empty(); }
		virtual bool Overlap(BoxCollider* other) = 0;
	private:
		
		BIND_UPDATE_ALL(Collider, Collider::UpdateAll, 90)
	};
} // namespace MG

