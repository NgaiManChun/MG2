#include "collider.h"
#include "boxCollider.h"
#include <algorithm>

namespace MG {


	void Collider::UpdateAll(Scene* scene, std::vector<Collider*>& components)
	{
		s_AABBs.erase(std::remove_if(s_AABBs.begin(), s_AABBs.end(), [](AABB& aabb) {
			if (!aabb.component->IsDestroyed()) {
				XMMATRIX world = XMMatrixTranspose(aabb.component->GetGameObject()->GetWorldMatrix().GetData());
				XMVECTOR center = world.r[3];

				// 3軸成分
				XMVECTOR axisX = XMVectorAbs(world.r[0]);
				XMVECTOR axisY = XMVectorAbs(world.r[1]);
				XMVECTOR axisZ = XMVectorAbs(world.r[2]);

				// ハーフサイズ0.5
				XMVECTOR extent = (axisX + axisY + axisZ) * 0.5f;
				aabb.min = center - extent;
				aabb.max = center + extent;

				aabb.component->m_OverlapColliders.clear();

				return false;
			}
			return true;
			}), s_AABBs.end());

		std::sort(s_AABBs.begin(), s_AABBs.end(), [](AABB& a, AABB& b) {
			return XMVectorGetX(a.min) < XMVectorGetX(b.min);
		});

		int size = static_cast<int>(s_AABBs.size());
		for (int current = 0; current < size; current++) {
			if (!s_AABBs[current].component->IsEnabled())
				continue;
			for (int forwrad = current + 1; forwrad < size; forwrad++) {
				if (!s_AABBs[forwrad].component->IsEnabled())
					continue;
				XMVECTOR out =
					XMVectorOrInt(
						XMVectorGreater(s_AABBs[forwrad].min, s_AABBs[current].max),
						XMVectorLess(s_AABBs[forwrad].max, s_AABBs[current].min)
					);

				// x軸でソートしているから
				// アウトしたら、うしろのピアも比較する必要なし
				if (XMVectorGetX(out))
					break;

				if (XMVectorGetY(out) || XMVectorGetZ(out))
					continue;

				switch (s_AABBs[current].shape)
				{
				case SHAPE_BOX:
					if (s_AABBs[forwrad].component->Overlap(static_cast<BoxCollider*>(s_AABBs[current].component))) {
						s_AABBs[current].component->m_OverlapColliders.push_back(s_AABBs[forwrad].component);
						s_AABBs[forwrad].component->m_OverlapColliders.push_back(s_AABBs[current].component);
					}
					break;
				default:
					break;
				}

				
			}
			
		}
		
	}
} // namespace MG

