#pragma once
#include <windows.h>

namespace MG {
	void Setup(HWND hWnd, const char* initScene, unsigned int initSceneWidth = 1280, unsigned int initSceneHeight = 720, unsigned int initFPS = 60);
	void Finalize();
} // namespace MG