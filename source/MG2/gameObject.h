// =======================================================
// gameObject.h
// 
// ゲームオブジェクトクラス
// =======================================================
#pragma once
#include "vector3.h"
#include "matrix4x4.h"
#include "dynamicMatrix.h"
#include "scene.h"
#include <vector>
#include <DirectXMath.h>
using namespace DirectX;

namespace MG {

	class Scene;
	class Component;

	class GameObject
	{
		friend Scene;
		friend Component;

	private:
		Vector3 m_Position{ 0.0f, 0.0f, 0.0f };
		Vector3 m_Rotation{ 0.0f, 0.0f, 0.0f };
		Vector3 m_Scale{ 1.0f, 1.0f, 1.0f };

		Vector3 m_Forward{ 0.0f, 0.0f, 1.0f };
		Vector3 m_Upper{ 0.0f, 1.0f, 0.0f };
		Vector3 m_Right{ 1.0f, 0.0f, 0.0f };

		DynamicMatrix m_WorldMatrix;

		GameObject* m_Parent = nullptr;
		std::vector<GameObject*> m_Children;
		std::vector<Component*> m_Components;
		size_t m_EmptyComponentIndex = 0;

		Scene* m_Scene = nullptr;
		size_t m_SceneGameObjectIndex = 0;

		bool m_Destroyed = false;
		bool m_Enabled = true;
		bool m_NeedUpdateDirection = true;

		GameObject(Vector3 position = Vector3::ZERO, Vector3 scale = Vector3::ONE, Vector3 rotation = Vector3::ZERO) :
			m_Position(position), m_Scale(scale), m_Rotation(rotation) 
		{
			m_WorldMatrix = DynamicMatrix::Create({});
		}

		~GameObject()
		{
			m_WorldMatrix.Release();
		}

		void Uninit();
		void UpdateDirection();

	public:
		const Vector3& GetPosition() const { return m_Position; }
		const Vector3& GetRotation() const { return m_Rotation; }
		const Vector3& GetScale() const { return m_Scale; }
		Scene* GetScene() const { return m_Scene; }
		bool IsDestroyed() { return m_Destroyed; }
		const std::vector<Component*>& GetComponents() { return m_Components; }
		GameObject* GetParent() const { return m_Parent; }
		void SetEnabled(bool enable) { m_Enabled = enable; }
		DynamicMatrix GetWorldMatrix() const { return m_WorldMatrix; }

		Vector3 GetWorldPosition() const 
		{
			XMMATRIX matrix = XMMatrixTranspose(m_WorldMatrix.GetData());
			return matrix.r[3];
		}

		const Vector3& GetRight()
		{
			if (m_NeedUpdateDirection) UpdateDirection();
			return m_Right;
		}

		const Vector3& GetForward()
		{
			if (m_NeedUpdateDirection) UpdateDirection();
			return m_Forward;
		}

		const Vector3& GetUpper()
		{
			if (m_NeedUpdateDirection) UpdateDirection();
			return m_Upper;
		}

		bool IsEnabled() const;
		void Destroy();
		bool SetParent(GameObject* newParent);
		void SetPosition(const Vector3& position);
		void SetRotation(const Vector3& rotation);
		void SetScale(const Vector3& scale);
		void UpdateWorldMatrix();
		void RemoveComponent(Component* component);

		template <typename COMPONENT, typename... Args>
		COMPONENT* AddComponent(Args&&... args)
		{
			if (!m_Destroyed && std::is_base_of<Component, COMPONENT>::value) {
				COMPONENT* component = Component::GetDestroyedComponent<COMPONENT>(m_Scene);
				if (component) {
					size_t index = component->m_Index;
					component->~COMPONENT();
					new (component) COMPONENT(std::forward<Args>(args)...);
					component->m_Index = index;
					component->m_Scene = m_Scene;
					Component::AddInitialize<COMPONENT>(m_Scene, component);
				}
				else {
					component = new COMPONENT(std::forward<Args>(args)...);
					Component::Add<COMPONENT>(m_Scene, component);
				}

				if (component) {
					Component* baseComponent = reinterpret_cast<Component*>(component);
					size_t size = m_Components.size();
					for (size_t i = m_EmptyComponentIndex; i < size; i++) {
						if (m_Components[i] == nullptr) {
							m_Components[i] = baseComponent;
							baseComponent->SetGameObject(this);
							m_EmptyComponentIndex = i + 1;
							return component;
						}
					}
					m_Components.push_back(baseComponent);
					baseComponent->SetGameObject(this);
					m_EmptyComponentIndex = size + 1;
					return component;
				}
			}
			return nullptr;
		}

		template <typename COMPONENT>
		COMPONENT* AddComponent(COMPONENT* component)
		{
			if (!m_Destroyed && std::is_base_of<Component, COMPONENT>::value) {
				Component* baseComponent = reinterpret_cast<Component*>(component);
				size_t size = m_Components.size();
				for (size_t i = m_EmptyComponentIndex; i < size; i++) {
					if (m_Components[i] == nullptr) {
						m_Components[i] = baseComponent;
						baseComponent->SetGameObject(this);
						m_EmptyComponentIndex = i + 1;
						return component;
					}
				}
				m_Components.push_back(baseComponent);
				baseComponent->SetGameObject(this);
				m_EmptyComponentIndex = size + 1;
				return component;
			}
			return component;
		}

		template <typename COMPONENT>
		COMPONENT* GetComponent()
		{
			for (auto component : m_Components) {
				COMPONENT* find = dynamic_cast<COMPONENT*>(component);
				if (find != nullptr) {
					return find;
				}
			}

			return nullptr;
		}
	};
} // namespace MG

