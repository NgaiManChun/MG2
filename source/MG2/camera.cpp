#include "camera.h"
#include "renderer.h"
#include "input.h"
#include "scene.h"
#include "MGUtility.h"

namespace MG {

	void Camera::Update()
	{
		/*return;
		GameObject* gameObject = GetGameObject();
		Vector3 forward = gameObject->GetForward();
		Vector3 right = gameObject->GetRight();

		float deltaTime = MGUtility::GetDeltaTime();;
		Vector3 position = gameObject->GetPosition();
		Vector3 rotation = gameObject->GetRotation();
		bool posChanged = false;
		bool rotChanged = false;
		if (Input::GetKeyPress('W')) {
			position += forward * 5.0f * deltaTime;
			posChanged = true;
		}
		if (Input::GetKeyPress('S')) {
			position -= forward * 5.0f * deltaTime;
			posChanged = true;
		}
		if (Input::GetKeyPress('A')) {
			position -= right * 5.0f * deltaTime;
			posChanged = true;
		}
		if (Input::GetKeyPress('D')) {
			position += right * 5.0f * deltaTime;
			posChanged = true;
		}
		if (posChanged) {
			gameObject->SetPosition(position);
		}

		if (Input::GetKeyPress(VK_UP)) {
			rotation.x -= 1.0f * deltaTime;
			rotChanged = true;
		}
		if (Input::GetKeyPress(VK_DOWN)) {
			rotation.x += 1.0f * deltaTime;
			rotChanged = true;
		}
		if (Input::GetKeyPress(VK_LEFT)) {
			rotation.y -= 1.0f * deltaTime;
			rotChanged = true;
		}
		if (Input::GetKeyPress(VK_RIGHT)) {
			rotation.y += 1.0f * deltaTime;
			rotChanged = true;
		}
		if (rotChanged) {
			gameObject->SetRotation(rotation);
		}

		*/

	}


	void Camera::Apply()
	{
		GameObject* gameObject = GetGameObject();
		XMVECTOR position = gameObject->GetPosition();
		XMVECTOR forward = gameObject->GetForward();
		XMMATRIX view = XMMatrixLookAtLH(position, position + forward, gameObject->GetUpper());

		XMMATRIX projection = XMMatrixPerspectiveFovLH(m_Angle, MGUtility::GetScreenRatio(), m_Near, m_Far);
		//projection = XMMatrixOrthographicOffCenterLH(-10.0f, 10.0f, -5.0f, 5.0f, 0.0f, 100.0f);

		XMMATRIX invViewRotation = XMMatrixInverse(nullptr, view);
		invViewRotation.r[3].m128_f32[0] = 0.0f;
		invViewRotation.r[3].m128_f32[1] = 0.0f;
		invViewRotation.r[3].m128_f32[2] = 0.0f;

		XMMATRIX invVp = XMMatrixInverse(nullptr, view * projection);

		XMVECTOR points[4] = {
			XMVectorSet(-1.0f,  1.0f, 1.0f, 0.0f),
			XMVectorSet( 1.0f,  1.0f, 1.0f, 0.0f),
			XMVectorSet(-1.0f, -1.0f, 1.0f, 0.0f),
			XMVectorSet( 1.0f, -1.0f, 1.0f, 0.0f)
		};
		points[0] = XMVector3TransformCoord(points[0], invVp);
		points[1] = XMVector3TransformCoord(points[1], invVp);
		points[2] = XMVector3TransformCoord(points[2], invVp);
		points[3] = XMVector3TransformCoord(points[3], invVp);

		Vector4 frustum[] = {
			XMVector3Normalize(XMVector3Cross(points[0] - position, points[2] - position)),
			XMVector3Normalize(XMVector3Cross(points[3] - position, points[1] - position)),
			XMVector3Normalize(XMVector3Cross(points[1] - position, points[0] - position)),
			XMVector3Normalize(XMVector3Cross(points[2] - position, points[3] - position))
		};


		Vector4 frustumPoints[] = {
			position,
			position,
			position,
			position
		};


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

	AABB Camera::GetProjectionAABB()
	{
		GameObject* gameObject = GetGameObject();
		XMVECTOR position = gameObject->GetPosition();
		XMVECTOR forward = gameObject->GetForward();
		XMMATRIX view = XMMatrixLookAtLH(position, position + forward, gameObject->GetUpper());

		XMMATRIX projection = XMMatrixPerspectiveFovLH(m_Angle, MGUtility::GetScreenRatio(), m_Near, min(m_Far, 10.0f));

		XMMATRIX invVp = XMMatrixInverse(nullptr, view * projection);

		XMVECTOR points[4] = {
			XMVectorSet( -1.0f,  1.0f, 1.0f, 0.0f),
			XMVectorSet(  1.0f,  1.0f, 1.0f, 0.0f),
			XMVectorSet( -1.0f, -1.0f, 1.0f, 0.0f),
			XMVectorSet(  1.0f, -1.0f, 1.0f, 0.0f)
		};
		points[0] = XMVector3TransformCoord(points[0], invVp);
		points[1] = XMVector3TransformCoord(points[1], invVp);
		points[2] = XMVector3TransformCoord(points[2], invVp);
		points[3] = XMVector3TransformCoord(points[3], invVp);

		XMVECTOR min = position;
		XMVECTOR max = position;
		for (int i = 0; i < 4; i++) {
			min = XMVectorMin(points[i], min);
			max = XMVectorMax(points[i], max);
		}

		return { min, max };
	}

} // namespace MG


