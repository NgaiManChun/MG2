#include "component.h"
#include <algorithm>

namespace MG {

	bool Component::AddStaticInitFunction(void (*function)(), int priority)
	{
		s_StaticInitFunctions.push_back(FUNCTION_PAIR{ function, priority });
		return true;
	}

	bool Component::AddStaticUninitFunction(void (*function)(), int priority)
	{
		s_StaticUninitFunctions.push_back(FUNCTION_PAIR{ function, priority });
		return true;
	}

	bool Component::AddInitAllFunction(void (*function)(Scene*), int priority)
	{
		s_InitAllFunctions.push_back(SCENE_FUNCTION_PAIR{ function, priority });
		return true;
	}

	bool Component::AddUpdateAllFunction(void (*function)(Scene*), int priority)
	{
		s_UpdateAllFunctions.push_back(SCENE_FUNCTION_PAIR{ function, priority });
		std::sort(s_UpdateAllFunctions.begin(), s_UpdateAllFunctions.end(), [](SCENE_FUNCTION_PAIR a, SCENE_FUNCTION_PAIR b) {
			return a.priority < b.priority;
		});
		return true;
	}

	bool Component::AddDrawAllFunction(void (*function)(Scene*), int priority)
	{
		s_DrawAllFunctions.push_back(SCENE_FUNCTION_PAIR{ function, priority });
		std::sort(s_DrawAllFunctions.begin(), s_DrawAllFunctions.end(), [](SCENE_FUNCTION_PAIR a, SCENE_FUNCTION_PAIR b) {
			return a.priority < b.priority;
		});
		return true;
	}

	bool Component::AddDestroyAllFunction(void (*function)(Scene*), int priority)
	{
		s_DestroyAllFunctions.push_back(SCENE_FUNCTION_PAIR{ function, priority });
		return true;
	}

	bool Component::AddReleaseDestroyedFunction(void (*function)(Scene*), int priority)
	{
		s_ReleaseDestroyedFunctions.push_back(SCENE_FUNCTION_PAIR{ function, priority });
		return true;
	}

} // namespace MG

