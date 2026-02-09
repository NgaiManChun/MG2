#include "scene.h"
#include "gameObject.h"
#include "component.h"
#include "camera.h"

namespace MG {

	GameObject* Scene::AddGameObject(Vector3 position, Vector3 scale, Vector3 rotation)
	{
		GameObject* gameObject = nullptr;
		
		// 再利用できるメモリを探す
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

		// 空いてるとこなければ、追加する
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
		// 親がないGameObjectを取得
		if (m_NeedUpdateRootGameObjects) {
			m_RootGameObjects.clear();
			for (GameObject* gameObject : m_GameObjects) {
				if (gameObject->m_Parent == nullptr) {
					m_RootGameObjects.push_back(gameObject);
				}
			}
		}

		// 親から階層的にワールド行列を計算
		if (m_NeedUpdateWorldMatrix) {
			for (GameObject* gameObject : m_RootGameObjects) {
				gameObject->UpdateWorldMatrix();
			}
			m_NeedUpdateWorldMatrix = false;
		}
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
	}

} // namespace MG


