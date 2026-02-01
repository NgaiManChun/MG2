#pragma once
#include "gameObject.h"

namespace MG {

	class Scene;
	class GameObject;

	class Component
	{
		friend GameObject;
	protected:
		GameObject* m_GameObject = nullptr;
		bool m_Initialized = false;
		bool m_Destroyed = false;
		bool m_Enabled = true;

		void SetGameObject(GameObject* gameObjecct) { m_GameObject = gameObjecct; }
	public:
		Component() {}
		Component(GameObject* gameObject) :m_GameObject(gameObject) {}
		virtual ~Component() {}

		virtual void Init() {};
		virtual void Uninit() {};
		virtual void Update() {};
		virtual void Draw() {};

		GameObject* GetGameObject() const { return m_GameObject; }
		const Vector3& GetPosition() const { return m_GameObject->m_Position; }
		const Vector3& GetRotation() const { return m_GameObject->m_Rotation; }
		const Vector3& GetScale() const { return m_GameObject->m_Scale; }
		const Vector3& GetRight() { return m_GameObject->GetRight(); }
		const Vector3& GetForward() { return m_GameObject->GetForward(); }
		const Vector3& GetUpper() { return m_GameObject->GetUpper(); }
		//XMMATRIX GetWorldMatrix() { return m_GameObject->GetWorldMatrix(); }
		//MatrixResource GetWorldMatrixResource() { return m_GameObject->GetWorldMatrixResource(); }
		void SetEnabled(bool enable) { m_Enabled = enable; }
		bool IsEnabled() const
		{
			return
				m_Initialized &&
				m_Enabled &&
				!m_Destroyed &&
				m_GameObject &&
				m_GameObject->IsEnabled();
		}
		void Initialize()
		{
			if (!m_Initialized && m_Enabled && !m_Destroyed && m_GameObject && m_GameObject->IsEnabled()) {
				Init();
				m_Initialized = true;
			}
		}
		void Destroy()
		{
			if (!m_Destroyed)
			{
				Uninit();
				m_Destroyed = true;
				m_Enabled = false;
				if (m_GameObject) {
					m_GameObject->RemoveComponent(this);
				}
				m_GameObject = nullptr;
			}
		}
		bool IsDestroyed() const { return m_Destroyed; }

	private:
		struct FUNCTION_PAIR {
			void (*function)();
			int priority;
		};

		struct SCENE_FUNCTION_PAIR {
			void (*function)(Scene*);
			int priority;
		};


		static inline std::vector<FUNCTION_PAIR> s_StaticInitFunctions{};

		static inline std::vector<FUNCTION_PAIR> s_StaticUninitFunctions{};

		static inline std::vector<SCENE_FUNCTION_PAIR> s_UpdateAllFunctions{};

		static inline std::vector<SCENE_FUNCTION_PAIR> s_DrawAllFunctions{};

		static inline std::vector<SCENE_FUNCTION_PAIR> s_DestroyAllFunctions{};

		static inline std::vector<SCENE_FUNCTION_PAIR> s_ReleaseDestroyedFunctions{};

		static bool AddStaticInitFunction(void (*function)(), int priority = 0)
		{
			s_StaticInitFunctions.push_back(FUNCTION_PAIR{ function, priority });
			return true;
		}

		static bool AddStaticUninitFunction(void (*function)(), int priority = 0)
		{
			s_StaticUninitFunctions.push_back(FUNCTION_PAIR{ function, priority });
			return true;
		}

		static bool AddUpdateAllFunction(void (*function)(Scene*), int priority = 0)
		{
			s_UpdateAllFunctions.push_back(SCENE_FUNCTION_PAIR{ function, priority });
			return true;
		}

		static bool AddDrawAllFunction(void (*function)(Scene*), int priority = 0)
		{
			s_DrawAllFunctions.push_back(SCENE_FUNCTION_PAIR{ function, priority });
			std::sort(s_DrawAllFunctions.begin(), s_DrawAllFunctions.end(), [](SCENE_FUNCTION_PAIR a, SCENE_FUNCTION_PAIR b) {
				return a.priority < b.priority;
			});
			return true;
		}

		static bool AddDestroyAllFunction(void (*function)(Scene*), int priority = 0)
		{
			s_DestroyAllFunctions.push_back(SCENE_FUNCTION_PAIR{ function, priority });
			return true;
		}

		static bool AddReleaseDestroyedFunction(void (*function)(Scene*), int priority = 0)
		{
			s_ReleaseDestroyedFunctions.push_back(SCENE_FUNCTION_PAIR{ function, priority });
			return true;
		}

	protected:
		template <typename COMPONENT>
		struct COMPONENT_VECTOR_PAIR {
			std::vector<COMPONENT*> components;
			size_t destoryedComponentIndex = 0;
		};

		template <typename COMPONENT>
		static inline std::unordered_map<Scene*, COMPONENT_VECTOR_PAIR<COMPONENT>> s_Components{};

	public:

		static void StaticInit()
		{
			for (auto& pair : s_StaticInitFunctions) {
				pair.function();
			}
		}

		static void StaticUninit()
		{
			for (auto& pair : s_StaticUninitFunctions) {
				pair.function();
			}
		}

		static void UpdateAll(Scene* scene)
		{
			for (auto& pair : s_UpdateAllFunctions) {
				pair.function(scene);
			}
		}

		static void DrawAll(Scene* scene)
		{
			for (auto& pair : s_DrawAllFunctions) {
				pair.function(scene);
			}
		}

		static void DestroyAll(Scene* scene)
		{
			for (auto& pair : s_DestroyAllFunctions) {
				pair.function(scene);
			}
		}

		static void ReleaseDestroyed(Scene* scene)
		{
			for (auto& pair : s_ReleaseDestroyedFunctions) {
				pair.function(scene);
			}
		}

		static inline std::unordered_map<std::string, Component* (*)()> s_InstanceFunctions{};

		template <typename COMPONENT>
		static COMPONENT* GetDestroyedComponent(Scene* scene)
		{
			COMPONENT* component = nullptr;

			auto& component_pair = Component::s_Components<COMPONENT>[scene];
			auto& sceneComponents = component_pair.components;
			size_t& destoryedComponentIndex = component_pair.destoryedComponentIndex;
			size_t size = sceneComponents.size();
			for (size_t i = destoryedComponentIndex; i < size; i++) {
				if (sceneComponents[i]->m_Destroyed) {
					destoryedComponentIndex = i;
					return sceneComponents[i];
				}
			}
			destoryedComponentIndex = size;
			return nullptr;
		}

		template <typename COMPONENT>
		static COMPONENT* GetComponent(Scene* scene)
		{
			auto& component_pair = Component::s_Components<COMPONENT>[scene];
			auto& sceneComponents = component_pair.components;
			for (auto component : sceneComponents) {
				if (!component->m_Destroyed) {
					return component;
				}
			}
			return nullptr;
		}

		template <typename COMPONENT>
		static std::vector<COMPONENT*> GetComponents(Scene* scene)
		{
			std::vector<COMPONENT*> result;
			auto& component_pair = Component::s_Components<COMPONENT>[scene];
			auto& sceneComponents = component_pair.components;
			for (auto component : sceneComponents) {
				if (!component->m_Destroyed) {
					result.push_back(component);
					//return component;
				}
			}
			return result;
		}

		template <typename COMPONENT, typename... Args>
		static COMPONENT* Create(Scene* scene, Args&&... args)
		{
			COMPONENT* component = new COMPONENT(std::forward<Args>(args)...);
			if (component) {
				auto& sceneComponents = Component::s_Components<COMPONENT>[scene].components;
				sceneComponents.push_back(component);
			}
			return component;
		}

		template <typename COMPONENT>
		static void Add(Scene* scene, COMPONENT* component)
		{
			if (component) {
				auto& sceneComponents = Component::s_Components<COMPONENT>[scene].components;
				sceneComponents.push_back(component);
			}
		}

		template <typename COMPONENT>
		static void UpdateAll(Scene* scene)
		{
			auto& component_pair = Component::s_Components<COMPONENT>[scene];
			auto& sceneComponents = component_pair.components;
			size_t& destoryedComponentIndex = component_pair.destoryedComponentIndex;
			size_t size = sceneComponents.size();
			for (ptrdiff_t i = static_cast<ptrdiff_t>(size) - 1; i >= 0; i--) {
				if (sceneComponents[i]) {
					COMPONENT& component = *sceneComponents[i];
					component.Initialize();
					if (component.IsEnabled()) {
						component.Update();
					}
					else if (component.IsDestroyed()) {
						destoryedComponentIndex = i;
					}
				}
			}
		}

		template <typename COMPONENT>
		static void DrawAll(Scene* scene)
		{
			auto& sceneComponents = Component::s_Components<COMPONENT>[scene].components;
			for (COMPONENT* p_Component : sceneComponents) {
				if (p_Component) {
					COMPONENT& component = *p_Component;
					if (component.IsEnabled()) {
						component.Draw();
					}
				}
			}
		}

		template <typename COMPONENT>
		static void DestroyAll(Scene* scene)
		{
			auto& sceneComponents = Component::s_Components<COMPONENT>[scene].components;
			for (COMPONENT* p_Component : sceneComponents) {
				if (p_Component) {
					p_Component->Destroy();
				}
			}
		}

		template <typename COMPONENT>
		static void ReleaseDestroyed(Scene* scene)
		{
			auto& sceneComponents = Component::s_Components<COMPONENT>[scene].components;
			sceneComponents.erase(
				std::remove_if(sceneComponents.begin(), sceneComponents.end(),
					[](COMPONENT* p_Component) {
						if (p_Component->IsDestroyed()) {
							delete p_Component;
							return true;
						}
						return false;
					}
				), sceneComponents.end()
			);
		}

		template <typename COMPONENT>
		class BindStaticInit {
		public:
			BindStaticInit(void (*function)(), int priority = 0) {
				static bool binded = Component::AddStaticInitFunction(function, priority);
			}
		};

		template <typename COMPONENT>
		class BindStaticUninit {
		public:
			BindStaticUninit(void (*function)(), int priority = 0) {
				static bool binded = Component::AddStaticUninitFunction(function, priority);
			}
		};

		template <typename COMPONENT>
		class BindUpdateAll {
		public:
			BindUpdateAll(void (*function)(Scene*) = Component::UpdateAll<COMPONENT>, int priority = 0) {
				static bool binded = Component::AddUpdateAllFunction(function, priority);
			}
		};

		template <typename COMPONENT>
		class BindDrawAll {
		public:
			BindDrawAll(void (*function)(Scene*) = Component::DrawAll<COMPONENT>, int priority = 0) {
				static bool binded = Component::AddDrawAllFunction(function, priority);
			}
		};

		template <typename COMPONENT>
		class BindDestroyAll {
		public:
			BindDestroyAll(void (*function)(Scene*) = Component::DestroyAll<COMPONENT>, int priority = 0) {
				static bool binded = Component::AddDestroyAllFunction(function, priority);
			}
		};

		template <typename COMPONENT>
		class BindRelaseDestroyed {
		public:
			BindRelaseDestroyed(void (*function)(Scene*) = Component::ReleaseDestroyed<COMPONENT>, int priority = 0) {
				static bool binded = Component::AddReleaseDestroyedFunction(function, priority);
			}
		};


	};
} // namespace MG

#define BIND_STATIC_INIT(COMPONENT, ...) \
static inline MG::Component::BindStaticInit<COMPONENT> m_BindStaticInit{ __VA_ARGS__ };

#define BIND_STATIC_UNINIT(COMPONENT, ...) \
static inline MG::Component::BindStaticUninit<COMPONENT> BindStaticUninit{ __VA_ARGS__ };

#define BIND_UPDATE_ALL(COMPONENT, ...) \
struct { MG::Component::BindUpdateAll<COMPONENT> m_BindUpdateAll{ __VA_ARGS__ }; };

#define BIND_DRAW_ALL(COMPONENT, ...) \
struct { MG::Component::BindDrawAll<COMPONENT> m_BindDrawAll{ __VA_ARGS__ }; };

#define BIND_DESTROY_ALL(COMPONENT, ...) \
struct { MG::Component::BindDestroyAll<COMPONENT> m_BindDestroyAll{ __VA_ARGS__ }; };

#define BIND_RELEASE_DESTROYED(COMPONENT, ...) \
struct { MG::Component::BindRelaseDestroyed<COMPONENT> m_BindRelaseDestroyed{ __VA_ARGS__ }; };

#define BIND_COMPONENT(COMPONENT) \
BIND_UPDATE_ALL(COMPONENT) \
BIND_DRAW_ALL(COMPONENT) \
BIND_DESTROY_ALL(COMPONENT) \
BIND_RELEASE_DESTROYED(COMPONENT)

#define BIND_COMPONENT_WITHOUT_DRAW(COMPONENT) \
BIND_UPDATE_ALL(COMPONENT) \
BIND_DESTROY_ALL(COMPONENT) \
BIND_RELEASE_DESTROYED(COMPONENT)