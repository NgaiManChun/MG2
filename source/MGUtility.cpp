#include "MGUtility.h"

namespace MG {

	void MGUtility::SetFPS(unsigned int fps) 
	{ 
		s_FPS = fps;
		s_PreferedFrameTime = 1.0f / s_FPS;
		while (s_FrameTime.size() > s_FPS) {
			s_FrameTime.pop();
		}
	}

	float MGUtility::GetLoadRate()
	{
		return std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - s_LastFrameTime).count() / s_PreferedFrameTime;
	}

	bool MGUtility::UpdateTime() {
		s_CurrentTime = std::chrono::high_resolution_clock::now();
		s_DeltaTime = std::chrono::duration<float>(s_CurrentTime - s_LastFrameTime).count();

		s_RunTimeMilliseconds = std::chrono::duration<float, std::milli>(std::chrono::high_resolution_clock::now() - s_StartTime).count();

		
		if (s_DeltaTime >= s_PreferedFrameTime) {

			s_FrameTime.push(s_CurrentTime);
			if (s_FrameTime.size() > s_FPS) {
				s_CurrentFPS = round(1.0f / std::chrono::duration<float>(s_CurrentTime - s_FrameTime.front()).count() * s_FPS);
				s_FrameTime.pop();
			}

			s_FrameCount++;

			s_LastFrameTime = s_CurrentTime;
			return true;
		}
		return false;
	}
} // namespace MG

