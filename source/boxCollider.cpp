#include "boxCollider.h"

namespace MG {
	
	void BoxCollider::UpdateAll(Scene* scene)
	{
		//struct AABB {
		//	//Vector3 center;
		//	//Vector3 extent;
		//	Vector3 min;
		//	Vector3 max;
		//	//float raidus;
		//	BoxCollider* component;
		//};

		//static std::vector<AABB> AABBs;
		//AABBs.clear();

		//auto& component_pair = Component::s_Components<BoxCollider>[scene];
		//auto& sceneComponents = component_pair.components;
		//for (BoxCollider* component : sceneComponents) {
		//	if (component->IsEnabled()) {
		//		XMMATRIX world = XMMatrixTranspose(component->GetGameObject()->GetWorldMatrix().GetData());
		//		XMVECTOR center = world.r[3];

		//		// 3軸成分
		//		XMVECTOR axisX = XMVectorAbs(world.r[0]);
		//		XMVECTOR axisY = XMVectorAbs(world.r[1]);
		//		XMVECTOR axisZ = XMVectorAbs(world.r[2]);

		//		// ハーフサイズ0.5
		//		XMVECTOR extent = (axisX + axisY + axisZ) * 0.5f;

		//		AABBs.push_back({
		//			center - extent,
		//			center + extent,
		//			component
		//		});
		//	}
		//	component->m_OverlapColliders.clear();
		//}

		///*AABB* minAABB = nullptr;
		//static std::vector<AABB> currentComparings;
		//currentComparings.clear();
		//for (size_t i = 0; i < size; i++) {
		//	if (minAABB == nullptr) {
		//		minAABB = (AABBs.data() + i);
		//		currentComparings.push_back(AABBs[i]);
		//	}
		//	else {
		//		if (AABBs[i].min.x < minAABB->max.x || AABBs[i].max.x < minAABB->max.x) {
		//			currentComparings.push_back(AABBs[i]);
		//		}
		//	}
		//}*/

		//std::sort(AABBs.begin(), AABBs.end(), []() {
		//	
		//});

		//int count = 0;
		//size_t size = AABBs.size();
		//for (size_t i = 0; i < size; i++) {
		//	for (size_t j = i + 1; j < size; j++) {
		//		if (AABBs[j].min.x > AABBs[i].max.x || AABBs[j].max.x < AABBs[i].min.x)
		//			continue;
		//		if (AABBs[j].min.y > AABBs[i].max.y || AABBs[j].max.y < AABBs[i].min.y)
		//			continue;
		//		if (AABBs[j].min.z > AABBs[i].max.z || AABBs[j].max.z < AABBs[i].min.z)
		//			continue;

		//		count++;

		//		if (AABBs[i].component->Overlap(AABBs[j].component)) {
		//			AABBs[i].component->m_OverlapColliders.push_back(AABBs[j].component);
		//			AABBs[j].component->m_OverlapColliders.push_back(AABBs[i].component);
		//		}


		//		/*bool outerSphereOverlap = 
		//			XMVectorGetX(XMVector3LengthSq(outerSpheres[i].center - outerSpheres[j].center)) <
		//			(outerSpheres[i].raidus + outerSpheres[j].raidus) * (outerSpheres[i].raidus + outerSpheres[j].raidus);
		//		if (outerSphereOverlap && outerSpheres[i].component->Overlap(outerSpheres[j].component)) {
		//			outerSpheres[i].component->m_OverlapColliders.push_back(outerSpheres[j].component);
		//			outerSpheres[j].component->m_OverlapColliders.push_back(outerSpheres[i].component);
		//		}*/
		//	}
		//}
		//int x = count;
	}
} // namespace MG

