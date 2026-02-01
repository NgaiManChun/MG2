#include "MGUtility.h"
#include "renderer.h"
#include "sceneManager.h"
#include "transformDivision.h"
#include "matrixDivision.h"
#include "vertexDivision.h"
#include "vertexIndexDivision.h"
#include "animationFollower.h"

namespace MG {
	void Setup(HWND hWnd, const char* initScene, unsigned int initSceneWidth, unsigned int initSceneHeight, unsigned int initFPS) {
		Renderer::Init(hWnd);
		MGUtility::SetScreenWidth(initSceneWidth);
		MGUtility::SetScreenHeight(initSceneHeight);
		MGUtility::SetFPS(initFPS);
		TransformDivision::Init();
		MatrixDivision::Init();
		VertexDivision::Init();
		VertexIndexDivision::Init();
		SceneManager::Init();
		SceneManager::AddScene(initScene);
	}

	void Finalize() {
		SceneManager::Uninit();
		VertexIndexDivision::Uninit();
		VertexDivision::Uninit();
		MatrixDivision::Uninit();
		TransformDivision::Uninit();
		AnimationFollower::Uninit();
		Renderer::Uninit();
	}
} // namespace MG