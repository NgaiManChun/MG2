#pragma once
#include <string>
#include <unordered_map>
#include <queue>

namespace MG {

	class SceneTransition;
	class Scene;

	class SceneManager
	{
	private:
		static inline std::vector<Scene*> s_Scenes{};
		static inline SceneTransition* s_Transition = nullptr;
	public:
		static void Init();
		static void Uninit();
		static void Update();

		static const std::string RegisterTransition(const std::string transitionName, SceneTransition* (*instanceFunction)(Scene& src, Scene& dest));

		static Scene* AddScene(const std::string sceneName);
		static SceneTransition* SetSceneTransition(const std::string sceneName);
		static bool IsInTransition() { return s_Transition; }

	};
} // namespace MG