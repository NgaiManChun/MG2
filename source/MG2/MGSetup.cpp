#include "MGUtility.h"
#include "renderer.h"
#include "sceneManager.h"

#include "boneDivision.h"
#include "dynamicIndexDivision.h"
#include "matrixDivision.h"
#include "transformDivision.h"
#include "vertexBoneWeightDivision.h"
#include "vertexDivision.h"
#include "vertexIndexDivision.h"
#include "animation.h"
#include "animationFollower.h"
#include "animationSet.h"
#include "dynamicMatrix.h"

#include "buffer.h"

namespace MG {
	void Setup(HWND hWnd, const char* initScene, unsigned int initSceneWidth, unsigned int initSceneHeight, unsigned int initFPS) {

		Renderer::Init(hWnd);

		MGUtility::SetScreenWidth(initSceneWidth);
		MGUtility::SetScreenHeight(initSceneHeight);
		MGUtility::SetFPS(initFPS);

		SceneManager::Init();
		SceneManager::AddScene(initScene);
	}

	void Finalize() {
		SceneManager::Uninit();

		// バッファ系
		BoneDivision::Uninit();
		DynamicIndexDivision::Uninit();
		MatrixDivision::Uninit();
		TransformDivision::Uninit();
		VertexBoneWeightDivision::Uninit();
		VertexDivision::Uninit();
		VertexIndexDivision::Uninit();
		Animation::Uninit();
		AnimationFollower::Uninit();
		AnimationSet::Uninit();
		DynamicMatrix::Uninit();
		Material::Uninit();

		Buffer::Uninit();

		Renderer::Uninit();
	}
} // namespace MG