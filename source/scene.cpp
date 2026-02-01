#include "scene.h"
#include <algorithm>
#include "gameObject.h"
#include "component.h"
#include "camera.h"

namespace MG {

	GameObject* Scene::AddGameObject(Vector3 position, Vector3 scale, Vector3 rotation)
	{
		GameObject* gameObject = nullptr;
		
		// çƒóòóp
		size_t size = m_GameObjects.size();
		for (size_t i = m_EmptyGameObjectIndex; i < size; i++) {
			gameObject = m_GameObjects[i];
			if (gameObject->m_Destroyed) {
				gameObject->~GameObject();
				new (gameObject) GameObject(position, scale, rotation);
				gameObject->m_Scene = this;
				gameObject->m_SceneGameObjectIndex = i;
				m_NeedUpdateRootGameObjects = true;
				m_NeedUpdateWorldMatrix = true;
				m_EmptyGameObjectIndex = i + 1;
				return gameObject;
			}
		}

		gameObject = new GameObject(position, scale, rotation);
		if (gameObject) {
			m_GameObjects.push_back(gameObject);
			gameObject->m_Scene = this;
			gameObject->m_SceneGameObjectIndex = size;
			m_NeedUpdateRootGameObjects = true;
			m_NeedUpdateWorldMatrix = true;
			m_EmptyGameObjectIndex = size + 1;
		}
		return gameObject;
	}

	void Scene::UpdateGameObjectWorlds()
	{
		if (m_NeedUpdateRootGameObjects) {
			m_RootGameObjects.clear();
			for (GameObject* gameObject : m_GameObjects) {
				if (gameObject->m_Parent == nullptr) {
					m_RootGameObjects.push_back(gameObject);
				}
			}
		}

		if (m_NeedUpdateWorldMatrix) {
			for (GameObject* gameObject : m_RootGameObjects) {
				gameObject->UpdateWorldMatrix();
			}
			m_NeedUpdateWorldMatrix = false;
		}
		

		/*for (GameObject* gameObject : m_GameObjects) {
			if (gameObject->m_NeedUpdateWorldMatrix) {
				gameObject->UpdateWorldMatrix();
			}
		}*/
	}

	void Scene::InitScene()
	{
		m_MainCamera = AddGameObject()->AddComponent<Camera>();
		m_Enabled = true;
		m_Initialized = true;
	}

	void Scene::UninitScene()
	{
		for (auto gameObject : m_GameObjects) {
			gameObject->Destroy();
			delete gameObject;
		}
		m_GameObjects.clear();
		Component::DestroyAll(this);
		Component::ReleaseDestroyed(this);
		m_MainCamera = nullptr;
		m_Enabled = false;

		/*Model::ReleaseScope(this);
		Animation::ReleaseScope(this);
		Texture::ReleaseScope(this);*/
	}

	void Scene::BeforeUpdate()
	{
		/*size_t size = m_GameObjects.size();
		for (ptrdiff_t i = static_cast<ptrdiff_t>(size) - 1; i >= 0; i--) {
			if (m_GameObjects[i]->m_Destroyed) {
				m_EmptyGameObjectIndex = i;
			}
			else {
				m_GameObjects[i]->m_AttributeModified = 0;
			}
		}*/
	}
} // namespace MG


