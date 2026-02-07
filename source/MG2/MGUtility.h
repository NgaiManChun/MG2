#pragma once
#include <queue>
#include <chrono>
#include <Windows.h>

#define RANDOM_T ((float)rand() / RAND_MAX)
#define RANDOM_SIGN ((rand() % 2) ? 1.0f : -1.0f)

#define SAFE_RELEASE(POINTER)	\
if (POINTER != nullptr) {		\
	POINTER->Release();			\
	POINTER = nullptr;			\
}

namespace MG {
	
	class MGUtility {
	private:
		
		static inline int s_ScreenWidth = 1280;
		static inline int s_ScreenHeight = 720;
		static inline float s_ScreenRatio = static_cast<float>(s_ScreenWidth) / s_ScreenHeight;
		static inline unsigned int s_FPS = 60;
		static inline float s_PreferedFrameTime = 1.0f / s_FPS;
		static inline std::queue<std::chrono::steady_clock::time_point> s_FrameTime{};
		static inline std::chrono::steady_clock::time_point s_StartTime = std::chrono::high_resolution_clock::now();
		static inline std::chrono::steady_clock::time_point s_LastFrameTime = std::chrono::high_resolution_clock::now();
		static inline std::chrono::steady_clock::time_point s_LastMonitorTime = std::chrono::high_resolution_clock::now();
		static inline std::chrono::steady_clock::time_point s_CurrentTime;
		static inline unsigned int s_FrameCount = 0;
		static inline float s_DeltaTime = 0.0f;
		static inline unsigned int s_CurrentFPS = 0;
		static inline unsigned int s_RunTimeMilliseconds = 0;


	public:
		static constexpr const float PI = 3.14159274f;

		static void SetFPS(unsigned int fps);

		static void SetScreenWidth(unsigned int width) { 
			s_ScreenWidth = width; 
			s_ScreenRatio = static_cast<float>(s_ScreenWidth) / s_ScreenHeight;
		}
		static void SetScreenHeight(unsigned int height) { 
			s_ScreenHeight = height;
			s_ScreenRatio = static_cast<float>(s_ScreenWidth) / s_ScreenHeight;
		}

		static int GetScreenWidth() { return s_ScreenWidth; }

		static int GetScreenHeight() { return s_ScreenHeight; }

		static float GetScreenRatio() { return s_ScreenRatio; }

		static float GetDeltaTime() { return s_DeltaTime; }

		static unsigned int GetRunTimeMilliseconds() { return s_RunTimeMilliseconds; }

		static unsigned int GetCurrentFPS() { return s_CurrentFPS; }

		static float GreferedFrameTime() { return s_PreferedFrameTime; }

		static bool UpdateTime();

		static float GetLoadRate();

	};

	
}