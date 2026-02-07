
#ifdef _DEBUG 
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


#endif

//static int AllocBreakPoint() {
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//	//_CrtSetBreakAlloc(169);
//
//	return 0;
//}
//static int _AllocBreakPoint = AllocBreakPoint();

#include <windows.h>
#include <assert.h>
#include <thread>
#include "input.h"
#include "MGSetup.h"
#include "MGUtility.h"
#include "sceneManager.h"

static constexpr const char* CLASS_NAME = "MGP";
static constexpr const char* WINDOW_NAME = "MGP";
static constexpr const char* INIT_SCENE = "TestScene";
static constexpr const UINT FPS = 60;
static constexpr const unsigned int SCREEN_WIDTH = 1280;
static constexpr const unsigned int SCREEN_HEIGHT = 720;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND g_Window;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	WNDCLASSEX wcex;
	{
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = 0;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = nullptr;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = nullptr;
		wcex.lpszMenuName = nullptr;
		wcex.lpszClassName = CLASS_NAME;
		wcex.hIconSm = nullptr;

		RegisterClassEx(&wcex);


		RECT rc = { 0, 0, (LONG)SCREEN_WIDTH, (LONG)SCREEN_HEIGHT };
		AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

		g_Window = CreateWindowEx(0, CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);
	}

	CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	srand(timeGetTime());

	Input::Init();
	MG::Setup(g_Window, INIT_SCENE, SCREEN_WIDTH, SCREEN_HEIGHT, FPS);
	

	ShowWindow(g_Window, nCmdShow);
	UpdateWindow(g_Window);

	timeBeginPeriod(1);
	
	static int overloadTime = 0;
	static unsigned int FPSShift[] = {
		60,
		50,
		40,
		30
	};
	static unsigned int FPSShiftIndex = 0;

	MSG msg;
	while(1)
	{
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
        }

		if (MG::MGUtility::UpdateTime()) {

			Input::Update();
			MG::SceneManager::Update();

			unsigned int currentFPS = MG::MGUtility::GetCurrentFPS();
			float loadRate = MG::MGUtility::GetLoadRate();
			if (loadRate > 0.8f) {
				if (overloadTime < 0)
					overloadTime = 0;
				overloadTime++;
				if (overloadTime > currentFPS * 0.25f) {
					overloadTime = 0;
					if (FPSShiftIndex < ARRAYSIZE(FPSShift) - 1) {
						FPSShiftIndex++;
						MG::MGUtility::SetFPS(FPSShift[FPSShiftIndex]);
					}
				}
			}
			else if(loadRate < 0.3f){
				if (overloadTime > 0)
					overloadTime = 0;
				overloadTime--;
				if (overloadTime < (0 - currentFPS)) {
					overloadTime = 0;
					if (FPSShiftIndex > 0) {
						FPSShiftIndex--;
						MG::MGUtility::SetFPS(FPSShift[FPSShiftIndex]);
					}
				}
			}
			else {
				overloadTime = 0;
			}

#if _DEBUG
			
			
			float deltaTime = MG::MGUtility::GetDeltaTime();
			std::string title = std::string("FPS: ") + 
				std::to_string(currentFPS) + 
				std::string("/") +
				std::to_string(FPSShift[FPSShiftIndex]) +
				std::string(" Load Rate: ") + 
				std::to_string(loadRate);
			SetWindowTextA(g_Window, title.c_str());
#endif
		}
	}

	timeEndPeriod(1);

	UnregisterClass(CLASS_NAME, wcex.hInstance);

	MG::Finalize();

	Input::Uninit();

	CoUninitialize();

	return (int)msg.wParam;
}




LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch(uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hWnd);
			break;
		}
		break;

	default:
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

