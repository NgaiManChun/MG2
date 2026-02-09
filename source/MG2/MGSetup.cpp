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
#include "material.h"
#include "mesh.h"
#include "model.h"
#include "modelAnimation.h"
#include "modelInstance.h"
#include "texture.h"

#include "buffer.h"

namespace MG {
	void Setup(HWND hWnd, const char* initScene, unsigned int initSceneWidth, unsigned int initSceneHeight, unsigned int initFPS) {

		Renderer::Init(hWnd);

		MGUtility::SetScreenWidth(initSceneWidth);
		MGUtility::SetScreenHeight(initSceneHeight);
		MGUtility::SetTargetFPS(initFPS);

		SceneManager::Init();
		SceneManager::AddScene(initScene);
	}

	void Finalize() {
		SceneManager::Uninit();

		// バッファ系
		Model::Uninit();
		ModelAnimation::Uninit();
		Mesh::Uninit();
		Material::Uninit();
		Texture::Uninit();
		ModelInstance::Uninit();
		Animation::Uninit();
		AnimationFollower::Uninit();
		AnimationSet::Uninit();
		BoneDivision::Uninit();
		VertexBoneWeightDivision::Uninit();
		DynamicIndexDivision::Uninit();
		MatrixDivision::Uninit();
		TransformDivision::Uninit();
		DynamicMatrix::Uninit();
		VertexDivision::Uninit();
		VertexIndexDivision::Uninit();
		Buffer::Uninit();

		Renderer::Uninit();
	}
} // namespace MG