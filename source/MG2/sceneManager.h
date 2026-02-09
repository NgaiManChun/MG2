// =======================================================
// sceneManager.h
// 
// シーンの管理クラス
// ゲーム全体の更新のエントリーポイント
// =======================================================
#pragma once
#include <string>
#include <vector>

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

		static Scene* AddScene(const std::string sceneName);
		static SceneTransition* SetSceneTransition(const std::string sceneName);
		static bool IsInTransition() { return s_Transition; }

	};
} // namespace MG