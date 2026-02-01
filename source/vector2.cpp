#include "vector2.h"
#include <cstdlib>

namespace MG {
	const Vector2 Vector2::ZERO = Vector2(0.0f, 0.0f);
	const Vector2 Vector2::ONE = Vector2(1.0f, 1.0f);

	Vector2 Vector2::GetRandom() {
		return {
			(float)rand() / RAND_MAX,
			(float)rand() / RAND_MAX
		};
	}

	Vector2 Vector2::GetSignedRandom() {
		return {
			(float)rand() / RAND_MAX * ((rand() % 2) ? 1.0f : -1.0f),
			(float)rand() / RAND_MAX * ((rand() % 2) ? 1.0f : -1.0f)
		};
	}
} // namespace MG

