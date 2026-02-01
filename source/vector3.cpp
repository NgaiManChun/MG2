#include "vector3.h"
#include <cstdlib>

namespace MG {
	const Vector3 Vector3::ZERO = Vector3(0.0f, 0.0f, 0.0f);
	const Vector3 Vector3::ONE = Vector3(1.0f, 1.0f, 1.0f);

	Vector3 Vector3::GetRandom() {
		return {
			(float)rand() / RAND_MAX,
			(float)rand() / RAND_MAX,
			(float)rand() / RAND_MAX,
		};
	}

	Vector3 Vector3::GetSignedRandom() {
		return {
			(float)rand() / RAND_MAX * ((rand() % 2) ? 1.0f : -1.0f),
			(float)rand() / RAND_MAX * ((rand() % 2) ? 1.0f : -1.0f),
			(float)rand() / RAND_MAX * ((rand() % 2) ? 1.0f : -1.0f),
		};
	}

	Vector3 Vector3::Max(const Vector3& a, const Vector3& b) {
		return {
			fmaxf(a.x, b.x),
			fmaxf(a.y, b.y),
			fmaxf(a.z, b.z)
		};
	}

	Vector3 Vector3::Min(const Vector3& a, const Vector3& b) {
		return {
			fminf(a.x, b.x),
			fminf(a.y, b.y),
			fminf(a.z, b.z)
		};
	}
} // namespace MG

