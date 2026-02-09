#include "camera.h"
#include "renderer.h"
#include "MGUtility.h"

namespace MG {

	void Camera::Apply()
	{
		GameObject* gameObject = GetGameObject();
		XMVECTOR position = gameObject->GetPosition();
		XMVECTOR forward = gameObject->GetForward();
		XMMATRIX view = XMMatrixLookAtLH(position, position + forward, gameObject->GetUpper());
		XMMATRIX projection = XMMatrixPerspectiveFovLH(m_Angle, MGUtility::GetScreenRatio(), m_Near, m_Far);

		// ViewProjectionの逆行列
		XMMATRIX invVp = XMMatrixInverse(nullptr, view * projection);

		// ビルボード用の逆行列
		XMMATRIX invViewRotation = XMMatrixInverse(nullptr, view);
		invViewRotation.r[3].m128_f32[0] = 0.0f;
		invViewRotation.r[3].m128_f32[1] = 0.0f;
		invViewRotation.r[3].m128_f32[2] = 0.0f;

		XMVECTOR points[4] = {
			XMVector3TransformCoord(XMVectorSet(-1.0f,  1.0f, 1.0f, 0.0f), invVp),
			XMVector3TransformCoord(XMVectorSet( 1.0f,  1.0f, 1.0f, 0.0f), invVp),
			XMVector3TransformCoord(XMVectorSet(-1.0f, -1.0f, 1.0f, 0.0f), invVp),
			XMVector3TransformCoord(XMVectorSet( 1.0f, -1.0f, 1.0f, 0.0f), invVp)
		};

		Vector4 frustum[] = {
			XMVector3Normalize(XMVector3Cross(points[0] - position, points[2] - position)),
			XMVector3Normalize(XMVector3Cross(points[3] - position, points[1] - position)),
			XMVector3Normalize(XMVector3Cross(points[1] - position, points[0] - position)),
			XMVector3Normalize(XMVector3Cross(points[2] - position, points[3] - position))
		};

		Vector4 frustumPoints[] = { position, position, position, position };

		// カメラ定数を設定
		CAMERA_CONSTANT camera = {};
		XMStoreFloat4x4(&camera.view, XMMatrixTranspose(view));
		XMStoreFloat4x4(&camera.projection, XMMatrixTranspose(projection));
		XMStoreFloat4x4(&camera.viewProjection, XMMatrixTranspose(view * projection));
		XMStoreFloat4x4(&camera.invViewProjection, XMMatrixTranspose(XMMatrixInverse(nullptr, view * projection)));
		XMStoreFloat4x4(&camera.invViewRotation, XMMatrixTranspose(invViewRotation));
		camera.cameraPosition = gameObject->GetPosition();
		camera.projectionFar = m_Far;
		memcpy(camera.frustum, frustum, sizeof(Vector4) * ARRAYSIZE(frustum));
		memcpy(camera.frustumPoints, frustumPoints, sizeof(Vector4) * ARRAYSIZE(frustumPoints));
		camera.forceLOD = m_ForceLOD;
		XMStoreFloat3(&camera.cameraForward, forward);
		Renderer::SetCamera(camera);
	}

} // namespace MG


