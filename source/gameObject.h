#pragma once
#include "vector3.h"
#include "matrix4x4.h"
#include "dynamicMatrix.h"
#include "scene.h"
#include <vector>
#include <algorithm>
#include <DirectXMath.h>
using namespace DirectX;

#define ADD_COMPONENT(T, ...) \
AddComponent<T>(new T(__VA_ARGS__))

//typedef unsigned short ATTRIBUTE_MODIFIED;
//
//const constexpr ATTRIBUTE_MODIFIED ATTRIBUTE_MODIFIED_PARENT = 0x1;
//const constexpr ATTRIBUTE_MODIFIED ATTRIBUTE_MODIFIED_ENABLE = 0x2;
//const constexpr ATTRIBUTE_MODIFIED ATTRIBUTE_MODIFIED_LAYER =  0x4;
//
//const constexpr ATTRIBUTE_MODIFIED ATTRIBUTE_MODIFIED_LOCAL_POSITION = 0x8;
//const constexpr ATTRIBUTE_MODIFIED ATTRIBUTE_MODIFIED_LOCAL_ROTATION = 0x10;
//const constexpr ATTRIBUTE_MODIFIED ATTRIBUTE_MODIFIED_LOCAL_SCALE = 0x20;
//
//const constexpr ATTRIBUTE_MODIFIED ATTRIBUTE_MODIFIED_WORLD_POSITION = 0x40;
//const constexpr ATTRIBUTE_MODIFIED ATTRIBUTE_MODIFIED_WORLD_ROTATION = 0x80;
//const constexpr ATTRIBUTE_MODIFIED ATTRIBUTE_MODIFIED_WORLD_SCALE = 0x100;



typedef unsigned short LAYER;

const constexpr LAYER LAYER_0	= 0x0000;
const constexpr LAYER LAYER_1	= 0x0001;
const constexpr LAYER LAYER_2	= 0x0002;
const constexpr LAYER LAYER_3	= 0x0004;
const constexpr LAYER LAYER_4	= 0x0008;
const constexpr LAYER LAYER_5	= 0x0010;
const constexpr LAYER LAYER_6	= 0x0020;
const constexpr LAYER LAYER_7	= 0x0040;
const constexpr LAYER LAYER_8	= 0x0080;
const constexpr LAYER LAYER_9	= 0x0100;
const constexpr LAYER LAYER_10	= 0x0200;
const constexpr LAYER LAYER_11	= 0x0400;
const constexpr LAYER LAYER_12	= 0x0800;
const constexpr LAYER LAYER_13	= 0x1000;
const constexpr LAYER LAYER_14	= 0x2000;
const constexpr LAYER LAYER_15	= 0x4000;
const constexpr LAYER LAYER_16	= 0x8000;

namespace MG {

	class Scene;
	class Component;

	class GameObject
	{
		friend Scene;
		friend Component;
	private:
		
		GameObject* m_Parent = nullptr;
		std::vector<GameObject*> m_Children;

		Vector3 m_Position{ 0.0f, 0.0f, 0.0f };
		Vector3 m_Rotation{ 0.0f, 0.0f, 0.0f };
		Vector3 m_Scale{ 1.0f, 1.0f, 1.0f };

		Vector3 m_Forward{ 0.0f, 0.0f, 1.0f };
		Vector3 m_Upper{ 0.0f, 1.0f, 0.0f };
		Vector3 m_Right{ 1.0f, 0.0f, 0.0f };

		DynamicMatrix m_WorldMatrix;

		std::vector<Component*> m_Components;
		size_t m_EmptyComponentIndex = 0;

		bool m_Destroyed = false;
		bool m_Enabled = true;
		bool m_NeedUpdateDirection = true;
		//bool m_NeedUpdateWorldMatrix = true;
		LAYER m_Layer = LAYER_0;

		//ATTRIBUTE_MODIFIED m_AttributeModified;
		Scene* m_Scene = nullptr;
		size_t m_SceneGameObjectIndex = 0;
		
		void Uninit();
		void UpdateDirection();

		GameObject(Vector3 position = Vector3::ZERO, Vector3 scale = Vector3::ONE, Vector3 rotation = Vector3::ZERO) :
			m_Position(position), m_Scale(scale), m_Rotation(rotation) {
			m_WorldMatrix = DynamicMatrix::Create({});
		}

		~GameObject()
		{
			m_WorldMatrix.Release();
		}

	public:
		const Vector3& GetPosition() const { return m_Position; }
		const Vector3& GetRotation() const { return m_Rotation; }
		const Vector3& GetScale() const { return m_Scale; }
		bool IsEnabled() const;
		LAYER GetLayer() const { return m_Layer; }
		Scene* GetScene() const;
		void Destroy();
		bool IsDestroyed() { return m_Destroyed; }

		const std::vector<Component*>& GetComponents()
		{
			return m_Components;
		}

		//ATTRIBUTE_MODIFIED GetAttributeModified() const { return m_AttributeModified; }

		bool SetParent(GameObject* newParent);

		void SetPosition(const Vector3& position);
		void SetRotation(const Vector3& rotation);
		void SetScale(const Vector3& scale);
		void SetEnabled(bool enable) { 
			m_Enabled = enable;
		}
		void SetLayer(const LAYER layer) 
		{
			m_Layer = layer;
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

		DynamicMatrix GetWorldMatrix() const 
		{
			return m_WorldMatrix;
		}

		void UpdateWorldMatrix();

		void RemoveComponent(Component* component);

		template <typename COMPONENT, typename... Args>
		COMPONENT* AddComponent(Args&&... args)
		{
			if (!m_Destroyed && std::is_base_of<Component, COMPONENT>::value) {
				COMPONENT* component = Component::GetDestroyedComponent<COMPONENT>(m_Scene);
				if (component) {
					component->~COMPONENT();
					new (component) COMPONENT(std::forward<Args>(args)...);
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

