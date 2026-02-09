// =======================================================
// MGUtility.h
// 
// メインループにおけるフレーム時間、FPS、
// 解像度情報を一元管理するためのユーティリティクラス
// =======================================================
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
		static inline unsigned int s_ScreenWidth = 1280;
		static inline unsigned int s_ScreenHeight = 720;
		static inline float s_ScreenRatio = static_cast<float>(s_ScreenWidth) / s_ScreenHeight;
		static inline unsigned int s_TargetFPS = 60;
		static inline float s_FrameBudgetTime = 1.0f / s_TargetFPS; // フレーム時間予算

		// 起動時刻
		static inline std::chrono::steady_clock::time_point s_StartTime = std::chrono::high_resolution_clock::now();

		// 前フレーム時刻
		static inline std::chrono::steady_clock::time_point s_LastFrameTime = std::chrono::high_resolution_clock::now();

		// 現フレーム時刻
		static inline std::chrono::steady_clock::time_point s_CurrentTime;

		// フレーム時刻キュー
		static inline std::queue<std::chrono::steady_clock::time_point> s_FrameTimes{};

		static inline float s_DeltaTime = 0.0f;
		static inline unsigned int s_CurrentFPS = 0;
		static inline unsigned int s_RunTimeMilliseconds = 0; // 稼働時間（ミリ秒）

	public:
		static unsigned int GetScreenWidth() { return s_ScreenWidth; }
		static unsigned int GetScreenHeight() { return s_ScreenHeight; }
		static float GetScreenRatio() { return s_ScreenRatio; }
		static float GetDeltaTime() { return s_DeltaTime; }
		static unsigned int GetRunTimeMilliseconds() { return s_RunTimeMilliseconds; }
		static unsigned int GetCurrentFPS() { return s_CurrentFPS; }
		static float GetFrameBudgetTime() { return s_FrameBudgetTime; }

		static void SetScreenWidth(unsigned int width) { 
			s_ScreenWidth = width; 
			s_ScreenRatio = static_cast<float>(s_ScreenWidth) / s_ScreenHeight;
		}

		static void SetScreenHeight(unsigned int height) { 
			s_ScreenHeight = height;
			s_ScreenRatio = static_cast<float>(s_ScreenWidth) / s_ScreenHeight;
		}

		static void SetTargetFPS(unsigned int fps)
		{
			s_TargetFPS = fps;
			s_FrameBudgetTime = 1.0f / s_TargetFPS;
			while (s_FrameTimes.size() > s_TargetFPS) {
				s_FrameTimes.pop();
			}
		}

		// フレーム時間予算使用率
		static float GetLoadRate()
		{
			return std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - s_LastFrameTime).count() / s_FrameBudgetTime;
		}

		// 時間モニタリング、メインループに使う
		// 戻り値：1フレーム分の時間経過したか
		static bool UpdateTime()
		{
			s_CurrentTime = std::chrono::high_resolution_clock::now();
			s_DeltaTime = std::chrono::duration<float>(s_CurrentTime - s_LastFrameTime).count();

			s_RunTimeMilliseconds = 
				static_cast<unsigned int>(std::chrono::duration<float, std::milli>(
						std::chrono::high_resolution_clock::now() - s_StartTime
					).count());

			if (s_DeltaTime >= s_FrameBudgetTime) {

				s_FrameTimes.push(s_CurrentTime);
				if (s_FrameTimes.size() > s_TargetFPS) {
					s_CurrentFPS = 
						static_cast<unsigned int>(
							round(
								1.0f / 
								std::chrono::duration<float>(s_CurrentTime - s_FrameTimes.front()).count() * 
								s_TargetFPS
							));
					s_FrameTimes.pop();
				}

				s_LastFrameTime = s_CurrentTime;
				return true;
			}
			return false;
		}

	};

}