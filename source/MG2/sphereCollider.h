#pragma once

#include "component.h"
#include "dataType.h"
#include "boxCollider.h"

namespace MG {

	// コライダー自身はサイズを持たず
	// 原点(0, 0, 0)、半径0.5を基準値として
	// GameObjectのTransformによって変形させる
	class SphereCollider : public Component {
		
	public:
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

			XMMATRIX selfTransform = GetGameObject()->GetWorldMatrix().GetData();
			XMMATRIX otherTransform = other->GetGameObject()->GetWorldMatrix().GetData();
			XMMATRIX invTransforms[2] =
			{
				XMMatrixTranspose(XMMatrixInverse(nullptr, selfTransform) * otherTransform),
				XMMatrixTranspose(XMMatrixInverse(nullptr, otherTransform) * selfTransform)
			};

			// SphereローカルにBoxを変形させる
			XMVECTOR points[8];
			for (int i = 0; i < 8; ++i) {
				points[i] = XMVector3Transform(corners[i], invTransforms[0]);
				if (XMVectorGetX(XMVector3LengthSq(points[i])) <= 0.5f * 0.5f)
					return true;
			}

			alignas(16) static const XMVECTOR minV = XMVectorSet(-0.5f, -0.5f, -0.5f, 0.0f);
			alignas(16) static const XMVECTOR maxV = XMVectorSet(0.5f, 0.5f, 0.5f, 0.0f);

			// BoxローカルSphere中心
			XMVECTOR sphereCenter = XMVector3Transform(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), invTransforms[1]);

			XMVECTOR closest = XMVectorClamp(sphereCenter, minV, maxV);

			closest = XMVector3Transform(closest, invTransforms[0]);
			if (XMVectorGetX(XMVector3LengthSq(closest)) <= 0.5f * 0.5f)
				return true;

			return false;

			XMVECTOR axisX = XMVector3Normalize(points[1] - points[0]);
			XMVECTOR axisY = XMVector3Normalize(points[2] - points[0]);
			XMVECTOR axisZ = XMVector3Normalize(points[4] - points[0]);

			// 6面の法線
			XMVECTOR normals[6] =
			{
				 axisX, -axisX,
				 axisY, -axisY,
				 axisZ, -axisZ
			};

			// 6面の基準点
			XMVECTOR refPoints[6] =
			{
				points[1],
				points[0],
				points[2],
				points[1],
				points[4],
				points[3]
			};


			for (int i = 0; i < 6; i++) {
				// 面まで最短距離
				float minDistance = XMVectorGetX(XMVector3Dot(refPoints[i], -normals[i]));

				if (abs(minDistance) > 0.5f)
					continue;

				// 面に向けて半径分伸ばした点をBoxローカルに移動させる
				Vector3 point = XMVector3Transform(-normals[i] * minDistance, invTransforms[1]);

				if (point.x <= 0.5f && point.y <= 0.5f && point.z <= 0.5f)
					return true;
			}

			return false;
		}
	private:
		BIND_COMPONENT_WITHOUT_DRAW(SphereCollider)
	};
} // namespace MG

