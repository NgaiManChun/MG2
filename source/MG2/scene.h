// =======================================================
// scene.h
// 
// シーンの基底クラス
// =======================================================
#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include "vector3.h"

// シーン登録用マクロ
#define REGISTER_SCENE(SCENE) \
static inline MG::Scene::Register<SCENE> m_Register{ #SCENE };

namespace MG {

	class GameObject;
	class Camera;
	class SceneTransition;
	class SceneManager;

	class Scene {
		friend SceneManager;
		friend SceneTransition;
		friend GameObject;

	private:
		std::vector<GameObject*> m_GameObjects; // 全GameObject
		std::vector<GameObject*> m_RootGameObjects; // 親がないGameObjectのみ
		size_t m_EmptyGameObjectIndex = 0;

		Camera* m_MainCamera;
		Vector3 m_Ambient = { 0.3f, 0.3f, 0.27f };
		Vector3 m_DirectLightColor = { 0.63f, 0.6f, 0.6f };
		Vector3 m_DirectLightDirection = Vector3::Normalize(Vector3{ 0.3f, -1.0f, 0.3f });
		
		bool m_Enabled = false;
		bool m_Initialized = false;
		bool m_InTransition = false; // シーン遷移中
		bool m_Destroying = false;
		bool m_NeedUpdateRootGameObjects = false;
		bool m_NeedUpdateWorldMatrix = false;

		void SetInTransition(const bool inTransition) { m_InTransition = inTransition; }

	public:
		// 継承用
		virtual void Init() {}
		virtual void Uninit() {}
		virtual void Update() {}
		virtual ~Scene() = default;

		void InitScene();
		void UninitScene();
		GameObject* AddGameObject(Vector3 position = Vector3::ZERO, Vector3 scale = Vector3::ONE, Vector3 rotation = Vector3::ZERO);

		// 全GameObjectのワールド行列を更新
		void UpdateGameObjectWorlds();

		bool IsInTransition() const { return m_InTransition; }
		bool IsEnabled() const { return m_Enabled && m_Initialized; }
		bool IsInitialized() const { return m_Initialized; }
		bool IsDestroying() const { return m_Destroying; }
		void SetDestroying() { m_Destroying = true; }
		Camera* GetMainCamera() const { return m_MainCamera; }
		const Vector3& GetAmbient() const { return m_Ambient; }
		void SetAmbient(Vector3& ambient) { m_Ambient = ambient; }
		const Vector3& GetDirectLightColor() const { return m_DirectLightColor; }
		void SetDirectLightColor(Vector3& directLightColor) { m_DirectLightColor = directLightColor; }
		const Vector3& GetDirectLightDirection() const { return m_DirectLightDirection; }
		void SetDirectLightDirection(Vector3& directLightDirection) { m_DirectLightDirection = directLightDirection; }

		template <typename COMPONENT>
		GameObject* GetGameObject()
		{
			for (auto gameObject : m_GameObjects) {
				for (auto component : gameObject->m_Components) {
					if (dynamic_cast<COMPONENT*>(component) != nullptr) {
						return gameObject;
					}
				}
			}
			return nullptr;
		}

		template <typename COMPONENT>
		std::vector<GameObject*> GetGameObjects()
		{
			std::vector<GameObject*> found;
			for (auto gameObject : m_GameObjects) {
				for (auto component : gameObject->m_Components) {
					if (dynamic_cast<COMPONENT*>(component) != nullptr) {
						found.push_back(gameObject);
					}
				}
			}
			return found;
		}

	private:
		// 登録されたシーン作成用関数
		static inline std::unordered_map<std::string, Scene* (*)()> s_InstanceFunctions{};

		static Scene* Create(std::string sceneName)
		{
			if (s_InstanceFunctions.count(sceneName) > 0) {
				return s_InstanceFunctions[sceneName]();
			}
			return nullptr;
		}

		static void Destroy(Scene*& scene)
		{
			if (scene) {
				scene->Uninit();
				scene->UninitScene();
				delete scene;
				scene = nullptr;
			}
		}

	public:
		template <typename SCENE>
		class Register {
		public:
			Register(std::string sceneName) {
				if (std::is_base_of<Scene, SCENE>::value) {
					static bool binded = [sceneName]() {
						s_InstanceFunctions[sceneName] = []() -> Scene* { return reinterpret_cast<Scene*>(new SCENE()); };
						return true; 
					}();
				}
			}
		};
	};

} // namespace MG
