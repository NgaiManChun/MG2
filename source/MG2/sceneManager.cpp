#include "sceneManager.h"
#include <algorithm>

#include "renderer.h"
#include "scene.h"
#include "sceneTransition.h"
#include "component.h"



namespace MG {


	void SceneManager::Init()
	{
		Component::StaticInit();
	}

	void SceneManager::Uninit()
	{
		Component::StaticUninit();
		if (s_Transition) {
			s_Transition->Uninit();
			s_Transition = nullptr;
		}

		for (Scene* scene : s_Scenes) {
			scene->Uninit();
			scene->UninitScene();
			delete scene;
		}
	}

	void SceneManager::Update()
	{
		SceneTransition* transition = s_Transition;
		if (transition) {
			// 未実装
		}
		
		Renderer::Begin();
		for (Scene* scenePtr : s_Scenes) {
			if (scenePtr) {
				Scene& scene = *scenePtr;
				if (scene.m_Enabled) {
					Component::InitAll(scenePtr);
					Component::UpdateAll(scenePtr);
					scenePtr->Update();
					if (transition) {
						//transition->Draw(scenePtr);
					}
					else {
						//
						LIGHT_CONSTANT light = {
							{ scenePtr->m_Ambient.x, scenePtr->m_Ambient.y, scenePtr->m_Ambient.z, 0.0f }
						};
						Renderer::SetLight(light);
						Component::DrawAll(scenePtr);
					}
				}
			}
		}
		Renderer::End();

		// 終了したシーンをクリーンアップ
		s_Scenes.erase(std::remove_if(s_Scenes.begin(), s_Scenes.end(), [](Scene*& scenePtr) {
			if (scenePtr && scenePtr->m_Destroying) {
				Scene::Destroy(scenePtr);
				return true;
			}
			return false;
		}), s_Scenes.end());
		
	}

	Scene* SceneManager::AddScene(const std::string sceneName)
	{
		Scene* scene = Scene::Create(sceneName);
		if (scene) {
			scene->InitScene();
			scene->Init();
			s_Scenes.push_back(scene);
		}
		return scene;
	}

	SceneTransition* SceneManager::SetSceneTransition(const std::string sceneName)
	{
		return nullptr;
	}
}

