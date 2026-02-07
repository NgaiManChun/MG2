#pragma once

#include "component.h"
#include "dataType.h"
#include "collider.h"

namespace MG {

	// コライダー自身はサイズを持たず
	// 原点(0, 0, 0)、ハーフサイズ(0.5, 0.5, 0.5)を基準値として
	// GameObjectのTransformによって変形させる
	class BoxCollider : public Collider {
	public:
		BoxCollider(size_t tags = 0) : Collider(tags) {}

		static void UpdateAll(Scene* scene);

		void Init() override {
			AddAABB();
		}
		
		bool Overlap(BoxCollider* other) override
		{
			alignas(16) static const XMVECTOR corners[8] =
			{
				XMVectorSet(-0.5f, -0.5f, -0.5f, 1.0f),
				XMVectorSet(0.5f, -0.5f, -0.5f, 1.0f),
				XMVectorSet(-0.5f,  0.5f, -0.5f, 1.0f),
				XMVectorSet(0.5f,  0.5f, -0.5f, 1.0f),
				XMVectorSet(-0.5f, -0.5f,  0.5f, 1.0f),
				XMVectorSet(0.5f, -0.5f,  0.5f, 1.0f),
				XMVectorSet(-0.5f,  0.5f,  0.5f, 1.0f),
				XMVectorSet(0.5f,  0.5f,  0.5f, 1.0f),
			};

			static const unsigned int edgeIndexes[12][2] =
			{
				{0, 1}, {1, 3}, {3, 2}, {2, 0},
				{4, 5}, {5, 7}, {7, 6}, {6, 4},
				{0, 4}, {1, 5}, {2, 6}, {3, 7}
			};

			static const float offset[2] = { -0.5f, 0.5f };

			XMMATRIX selfTransform = GetGameObject()->GetWorldMatrix().GetData();
			XMMATRIX otherTransform = other->GetGameObject()->GetWorldMatrix().GetData();
			XMMATRIX invTransforms[2] =
			{
				// 自分のローカル空間
				XMMatrixTranspose(XMMatrixInverse(nullptr, selfTransform) * otherTransform),

				// 相手のローカル空間
				XMMatrixTranspose(XMMatrixInverse(nullptr, otherTransform) * selfTransform)
			};

			for (int index = 0; index < 2; ++index)
			{
				// 8点をローカル空間に変換
				XMVECTOR points[8];
				for (int i = 0; i < 8; ++i) {
					points[i] = XMVector3Transform(corners[i], invTransforms[index]);
					Vector3 point = XMVectorAbs(points[i]);
					if (point.x <= 0.5f &&
						point.y <= 0.5f &&
						point.z <= 0.5f) {
						return true;
					}
				}

				// 12エッジ VS 6面
				for (int i = 0; i < 12; ++i)
				{
					XMVECTOR& xmA = points[edgeIndexes[i][0]];
					XMVECTOR& xmB = points[edgeIndexes[i][1]];
					float aAxisValue[3];
					float bAxisValue[3];
					XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(aAxisValue), xmA);
					XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(bAxisValue), xmB);

					for (int axis = 0; axis < 3; axis++) {
						for (unsigned int sign = 0; sign < 2; sign++) {
							float aValue = aAxisValue[axis] - offset[sign];
							float bValue = bAxisValue[axis] - offset[sign];
							if (aValue > 0.0f != bValue > 0.0f) {

								// 交点
								float t = aValue / (aValue - bValue);
								float hitValue[3];
								XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(hitValue), XMVectorAbs(xmA + (xmB - xmA) * t));

								// 面を貫通してるか
								if (hitValue[(axis + 1) % 3] <= 0.5f && hitValue[(axis + 2) % 3] <= 0.5f)
									return true;
							}
						}
					}
				}
			}
			
			return false;
		}
	private:
		//BIND_UPDATE_ALL(BoxCollider, BoxCollider::UpdateAll, 90)
		BIND_COMPONENT_WITHOUT_DRAW(BoxCollider)
	};
} // namespace MG

