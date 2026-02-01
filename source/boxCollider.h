#pragma once

#include "component.h"
#include "dataType.h"

namespace MG {

	// コライダー自身はサイズを持たず
	// 原点(0, 0, 0)、幅(1, 1, 1)を基準値として
	// GameObjectのTransformによって変形させる
	class BoxCollider : public Component {
		BIND_COMPONENT_WITHOUT_DRAW(BoxCollider)
	public:
		AABB GetAABB()
		{
			XMVECTOR worldCenter = GetPosition();

			XMVECTOR localExtent = XMVectorSet(0.5f, 0.5f, 0.5f, 1.0f);

			localExtent *= XMVectorAbs(GetScale());

			XMMATRIX absR = Matrix4x4::RotatingMatrix(GetRotation());
			absR.r[0] = XMVectorAbs(absR.r[0]);
			absR.r[1] = XMVectorAbs(absR.r[1]);
			absR.r[2] = XMVectorAbs(absR.r[2]);
			absR.r[3] = XMVectorZero();

			XMVECTOR worldExtent = XMVectorSet(
				XMVectorGetX(XMVector3Dot(absR.r[0], localExtent)),
				XMVectorGetX(XMVector3Dot(absR.r[1], localExtent)),
				XMVectorGetX(XMVector3Dot(absR.r[2], localExtent)),
				0.0f
			);

			return {
				worldCenter - worldExtent,
				worldCenter + worldExtent
			};
		}
		bool Overlap(BoxCollider* other)
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

			// 自分の反転transform相手に掛けることによって、
			// 自分を原点(0, 0, 0)で幅(1, 1, 1)の立方体に見立てて
			// 固定軸で判定をする
			XMMATRIX selfTransform = GetGameObject()->GetWorldMatrix().GetData();
			XMMATRIX otherTransform = other->GetGameObject()->GetWorldMatrix().GetData();
			XMMATRIX invTransforms[2] =
			{
				XMMatrixTranspose(XMMatrixInverse(nullptr, selfTransform) * otherTransform),
				XMMatrixTranspose(XMMatrixInverse(nullptr, otherTransform) * selfTransform)
			};
			for (int index = 0; index < 2; ++index)
			{
				XMVECTOR points[8];
				for (int i = 0; i < 8; ++i) {
					points[i] = XMVector3Transform(corners[i], invTransforms[index]);
				}

				for (int i = 0; i < 8; ++i) {
					Vector3 point = XMVectorAbs(points[i]);
					if (point.x <= 0.5f &&
						point.y <= 0.5f &&
						point.z <= 0.5f) {
						return true;
					}
				}

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
	};
} // namespace MG

