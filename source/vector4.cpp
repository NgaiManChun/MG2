#include "vector4.h"
#include <cstdlib>

namespace MG {
	const Vector4 Vector4::ZERO = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	const Vector4 Vector4::ONE = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	const RGBA RGBA::WHITE = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	const RGBA RGBA::BLACK = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
	const Quaternion Quaternion::IDENTITY = Vector4(0.0f, 0.0f, 0.0f, 1.0f);

	
	Vector4 Vector4::GetRandom() {
		return {
			(float)rand() / RAND_MAX,
			(float)rand() / RAND_MAX,
			(float)rand() / RAND_MAX,
			(float)rand() / RAND_MAX
		};
	}

	Vector4 Vector4::GetSignedRandom() {
		return {
			(float)rand() / RAND_MAX * ((rand() % 2) ? 1.0f : -1.0f),
			(float)rand() / RAND_MAX * ((rand() % 2) ? 1.0f : -1.0f),
			(float)rand() / RAND_MAX * ((rand() % 2) ? 1.0f : -1.0f),
			(float)rand() / RAND_MAX * ((rand() % 2) ? 1.0f : -1.0f)
		};
	}

	Vector4 Vector4::Max(const Vector4& a, const Vector4& b) {
		return {
			fmaxf(a.x, b.x),
			fmaxf(a.y, b.y),
			fmaxf(a.z, b.z),
			fmaxf(a.w, b.w)
		};
	}

	Vector4 Vector4::Min(const Vector4& a, const Vector4& b) {
		return {
			fminf(a.x, b.x),
			fminf(a.y, b.y),
			fminf(a.z, b.z),
			fminf(a.w, b.w)
		};
	}

} // namespace MG

