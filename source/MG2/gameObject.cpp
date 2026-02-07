#include "gameObject.h"
#include "component.h"
#include "renderer.h"

namespace MG {

	bool GameObject::IsEnabled() const
	{
		return
			m_Enabled &&
			!m_Destroyed &&
			(!m_Parent || m_Parent->IsEnabled());
	}

	void GameObject::Uninit()
	{
		for (auto component : m_Components) {
			component->SetGameObject(nullptr);
		}
		m_Components.clear();
	}

	void GameObject::Destroy()
	{
		m_Destroyed = true;
		for (auto component : m_Components) {
			component->SetGameObject(nullptr);
			component->Destroy();
		}
		m_Components.clear();

		for (auto child : m_Children) {
			child->Destroy();
		}
		m_Children.clear();
		
		m_Parent = nullptr;
		m_Scene->m_NeedUpdateRootGameObjects = true;
		if (m_SceneGameObjectIndex < m_Scene->m_EmptyGameObjectIndex) {
			m_Scene->m_EmptyGameObjectIndex = m_SceneGameObjectIndex;
		}
		
	}

	bool GameObject::SetParent(GameObject* newParent)
	{
		if (m_Parent == newParent)
			return false;

		// 循環チェック
		if (newParent) {
			GameObject* p = newParent;
			while (p) {
				if (p == this)
					return false;
				p = p->m_Parent;
			}
		}

		// 元の親から外す
		if (m_Parent) {
			auto& siblings = m_Parent->m_Children;
			auto it = std::find(siblings.begin(), siblings.end(), this);
			if (it != siblings.end())
				siblings.erase(it);
		}

		// 新しい親に追加
		GameObject* oldParent = m_Parent;
		m_Parent = newParent;
		if (newParent) {
			newParent->m_Children.push_back(this);
		}

		// シーンに通知
		if (m_Scene) {
			if ((oldParent == nullptr && newParent != nullptr) ||
				(oldParent != nullptr && newParent == nullptr)) {
				m_Scene->m_NeedUpdateRootGameObjects = true;
				m_Scene->m_NeedUpdateWorldMatrix = true;
			}
		}
		return true;
	}

	void GameObject::SetPosition(const Vector3& position) {
		m_Position = position;
		m_Scene->m_NeedUpdateWorldMatrix = true;
	}
	void GameObject::SetRotation(const Vector3& rotation) {
		m_Rotation = rotation;
		m_NeedUpdateDirection = true;
		m_Scene->m_NeedUpdateWorldMatrix = true;
	}
	void GameObject::SetScale(const Vector3& scale) {
		m_Scale = scale;
		m_Scene->m_NeedUpdateWorldMatrix = true;
	}

	void GameObject::UpdateWorldMatrix()
	{
		if (m_NeedUpdateDirection) UpdateDirection();

		XMMATRIX matrix =
			XMMatrixTranspose(
				XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z) *
				//XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z) *
				//XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z)
				XMMATRIX(m_Right, m_Upper, m_Forward, XMVectorSet(m_Position.x, m_Position.y, m_Position.z, 1.0f))
			);
		if (m_Parent) {
			matrix = m_Parent->m_WorldMatrix.GetData() * matrix;
		}
		m_WorldMatrix.SetData(matrix);

		for (auto child : m_Children) {
			child->UpdateWorldMatrix();
		}

		/*if (m_NeedUpdateWorldMatrix) {
			
			m_NeedUpdateWorldMatrix = false;
		}*/
		
	}

	void GameObject::RemoveComponent(Component* component)
	{
		size_t size = m_Components.size();
		for (size_t i = 0; i < size; i++) {
			if (m_Components[i] == component) {
				m_Components[i]->SetGameObject(nullptr);
				m_Components[i] = nullptr;
				m_EmptyComponentIndex = i;
				break;
			}
		}
	}

	void GameObject::UpdateDirection()
	{
		XMMATRIX matrix = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
		
		m_Right = XMVector3Normalize(matrix.r[0]);
		m_Upper = XMVector3Normalize(matrix.r[1]);
		m_Forward = XMVector3Normalize(matrix.r[2]);

		m_NeedUpdateDirection = false;
	}

	
} // namespace MG

