#include "component.h"
#include <algorithm>

namespace MG {

	void Component::StaticInit()
	{
		if (!s_SortedStaticInitFunctions) {
			std::sort(s_StaticInitFunctions.begin(), s_StaticInitFunctions.end(), [](FUNCTION_PAIR a, FUNCTION_PAIR b) {
				return a.priority < b.priority;
			});
		}

		for (size_t i = 0; i < s_StaticInitFunctions.size(); i++) {
			s_StaticInitFunctions[i].function();
		}
	}

	void Component::StaticUninit()
	{
		if (!s_SortedStaticUninitFunctions) {
			std::sort(s_StaticUninitFunctions.begin(), s_StaticUninitFunctions.end(), [](FUNCTION_PAIR a, FUNCTION_PAIR b) {
				return a.priority < b.priority;
			});
		}

		for (size_t i = 0; i < s_StaticUninitFunctions.size(); i++) {
			s_StaticUninitFunctions[i].function();
		}
	}

	void Component::InitAll(Scene* scene)
	{
		if (!s_SortedInitAllFunctions) {
			std::sort(s_InitAllFunctions.begin(), s_InitAllFunctions.end(), [](SCENE_FUNCTION_PAIR a, SCENE_FUNCTION_PAIR b) {
				return a.priority < b.priority;
			});
		}

		for (size_t i = 0; i < s_InitAllFunctions.size(); i++) {
			s_InitAllFunctions[i].function(scene);
		}
	}

	void Component::UpdateAll(Scene* scene)
	{
		if (!s_SortedUpdateAllFunctions) {
			std::sort(s_UpdateAllFunctions.begin(), s_UpdateAllFunctions.end(), [](SCENE_FUNCTION_PAIR a, SCENE_FUNCTION_PAIR b) {
				return a.priority < b.priority;
			});
		}

		for (size_t i = 0; i < s_UpdateAllFunctions.size(); i++) {
			s_UpdateAllFunctions[i].function(scene);
		}
	}

	void Component::DrawAll(Scene* scene)
	{
		if (!s_SortedDrawAllFunctions) {
			std::sort(s_DrawAllFunctions.begin(), s_DrawAllFunctions.end(), [](SCENE_FUNCTION_PAIR a, SCENE_FUNCTION_PAIR b) {
				return a.priority < b.priority;
			});
		}

		for (size_t i = 0; i < s_DrawAllFunctions.size(); i++) {
			s_DrawAllFunctions[i].function(scene);
		}
	}

	void Component::DestroyAll(Scene* scene)
	{
		if (!s_SortedDestroyAllFunctions) {
			std::sort(s_DestroyAllFunctions.begin(), s_DestroyAllFunctions.end(), [](SCENE_FUNCTION_PAIR a, SCENE_FUNCTION_PAIR b) {
				return a.priority < b.priority;
			});
		}

		for (size_t i = 0; i < s_DestroyAllFunctions.size(); i++) {
			s_DestroyAllFunctions[i].function(scene);
		}
	}

	void Component::ReleaseDestroyed(Scene* scene)
	{
		if (!s_SortedReleaseDestroyedFunctions) {
			std::sort(s_ReleaseDestroyedFunctions.begin(), s_ReleaseDestroyedFunctions.end(), [](SCENE_FUNCTION_PAIR a, SCENE_FUNCTION_PAIR b) {
				return a.priority < b.priority;
			});
		}

		for (size_t i = 0; i < s_ReleaseDestroyedFunctions.size(); i++) {
			s_ReleaseDestroyedFunctions[i].function(scene);
		}
	}

} // namespace MG

