#pragma once
#include <math.h>

namespace MG {
	class Vector2 {
	public:
		float x, y;
		Vector2(const Vector2& a) : x(a.x), y(a.y) {}
		Vector2(float nx = 0.0f, float ny = 0.0f) : x(nx), y(ny) {}

		Vector2& operator =(const Vector2& a) {
			x = a.x; y = a.y;
			return *this;
		}

		bool operator ==(const Vector2& a) {
			return x == a.x && y == a.y;
		}

		bool operator !=(const Vector2& a) {
			return x != a.x || y != a.y;
		}

		void Zero() { x = y = 0.0f; }

		Vector2 operator -() const { return Vector2(-x, -y); }

		Vector2 operator +(const Vector2& a) const {
			return Vector2(x + a.x, y + a.y);
		}

		Vector2 operator -(const Vector2& a) const {
			return Vector2(x - a.x, y - a.y);
		}

		Vector2 operator *(float a) const {
			return Vector2(x * a, y * a);
		}

		Vector2 operator /(float a) const {
			float oneOverA = 1.0f / a;
			return Vector2(x * oneOverA, y * oneOverA);
		}

		Vector2& operator +=(const Vector2& a) {
			x += a.x;
			y += a.y;
			return *this;
		}

		Vector2& operator -=(const Vector2& a) {
			x -= a.x;
			y -= a.y;
			return *this;
		}

		Vector2& operator *=(float a) {
			x *= a;
			y *= a;
			return *this;
		}

		Vector2& operator /=(float a) {
			float oneOverA = 1.0f / a;
			x *= oneOverA;
			y *= oneOverA;
			return *this;
		}

		void Normalize() {
			float magSq = x * x + y * y;
			if (magSq > 0.0f) {
				float oneOverMag = 1.0f / sqrtf(magSq);
				x *= oneOverMag;
				y *= oneOverMag;
			}
		}

		float Length() const
		{
			return sqrtf(x * x + y * y);
		}

		static Vector2 Normalize(const Vector2& v) {
			float length = sqrtf((v.x * v.x) + (v.y * v.y));
			if (length > 0)	length = 1 / length;
			return {
			  v.x * length,
			  v.y * length
			};
		}

		static float Dot(const Vector2& a, const Vector2& b) {
			return a.x * b.x + a.y * b.y;
		}

		static Vector2 Hermite(const Vector2 p0, const Vector2 v0, const Vector2 p1, const Vector2 v1, const float t) {
			return p0 * (2.0f * t * t * t - 3.0f * t * t + 1.0f) +
				p1 * (-2.0f * t * t * t + 3.0f * t * t) +
				v0 * (t * t * t - 2.0f * t * t + t) +
				v1 * (t * t * t - t * t);
		}

		static Vector2 Bezier3(const Vector2& p0, const Vector2& p1, const Vector2& p2, const Vector2& p3, const float& t) {
			return
				p0 * powf(1.0f - t, 3) +
				p1 * 3.0f * powf(1.0f - t, 2) * t +
				p2 * 3.0f * (1.0f - t) * t * t +
				p3 * t * t * t;
		}
	};
} // namespace MG