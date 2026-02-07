#pragma once

#include "vector3.h"
#include "vector4.h"
//#include "quaternion.h"



namespace MG {
	struct Matrix4x4 {
	private:
		union {
			XMFLOAT4X4 xmfloat;
			float m[4][4];     
		};

	public:

		// 16個のfloatを行優先で代入するコンストラクタ
		Matrix4x4(float v00, float v01, float v02, float v03,
			float v10, float v11, float v12, float v13,
			float v20, float v21, float v22, float v23,
			float v30, float v31, float v32, float v33) 
			: xmfloat(v00, v01, v02, v03,
				v10, v11, v12, v13,
				v20, v21, v22, v23,
				v30, v31, v32, v33) {
		}

		Matrix4x4() { XMStoreFloat4x4(&xmfloat, XMMatrixIdentity()); }
		Matrix4x4(const XMFLOAT4X4& mat) : xmfloat(mat) {}
		Matrix4x4(const XMMATRIX& mat) { XMStoreFloat4x4(&xmfloat, mat); }

		operator XMFLOAT4X4() const { return xmfloat; }
		operator XMMATRIX() const { return XMLoadFloat4x4(&xmfloat); }

		float* operator[](int row) { return m[row]; }
		const float* operator[](int row) const { return m[row]; }

		XMMATRIX operator*(const Matrix4x4& other) const {
			XMMATRIX a = XMLoadFloat4x4(&xmfloat);
			XMMATRIX b = XMLoadFloat4x4(&other.xmfloat);
			return XMMatrixMultiply(a, b);
		}

		XMMATRIX operator*(const float other) const {
			XMMATRIX a = XMLoadFloat4x4(&xmfloat);
			return a * other;
		}

		bool operator==(const Matrix4x4& other) const
		{
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					if (m[i][j] != other.m[i][j]) {
						return false;
					}
				}
			}
			return true;
		}

		Matrix4x4& operator*=(const Matrix4x4& other) {
			*this = *this * other;
			return *this;
		}

		Matrix4x4& operator*=(const float other) {
			*this = *this * other;
			return *this;
		}

		XMVECTOR TransformNormal(const Vector3& v) const noexcept {
			XMMATRIX mat = XMLoadFloat4x4(&xmfloat);
			return XMVector3TransformNormal(v, mat);
		}

		XMMATRIX Inverse() const noexcept {
			XMMATRIX mat = XMLoadFloat4x4(&xmfloat);
			return XMMatrixInverse(nullptr, mat);
		}

		static XMMATRIX ScalingMatrix(const Vector3& v) {
			return XMMatrixScaling(v.x, v.y, v.z);
		}

		static XMMATRIX RotatingMatrix(const Vector3& euler) {
			return XMMatrixRotationRollPitchYaw(euler.x, euler.y, euler.z);
		}

		static XMMATRIX RotatingMatrix(const Quaternion& quaternion) {
			return XMMatrixRotationQuaternion(quaternion);
		}

		static XMMATRIX RotatingMatrix(const XMVECTOR& quaternion) {
			return XMMatrixRotationQuaternion(quaternion);
		}

		static XMMATRIX TranslatingMatrix(const Vector3& v) {
			return XMMatrixTranslation(v.x, v.y, v.z);
		}

		static XMMATRIX SRT(const Vector3& scale, const Quaternion& rotation, const Vector3& translate)
		{
			return 
				XMMatrixScaling(scale.x, scale.y, scale.z) *
				XMMatrixRotationQuaternion(rotation) *
				XMMatrixTranslation(translate.x, translate.y, translate.z);
		}

		static XMMATRIX SRT(const Vector3& scale, const Vector3& euler, const Vector3& translate)
		{
			return
				XMMatrixScaling(scale.x, scale.y, scale.z) *
				XMMatrixRotationRollPitchYaw(euler.x, euler.y, euler.z) *
				XMMatrixTranslation(translate.x, translate.y, translate.z);
		}

		static XMMATRIX TransposeMatrix(const Matrix4x4& mat) {
			return XMMatrixTranspose(mat);
		}

		static XMMATRIX TransposeMatrix(const XMMATRIX& mat) {
			return XMMatrixTranspose(mat);
		}

		size_t Hash() const
		{
			const unsigned char* data = reinterpret_cast<const unsigned char*>(&xmfloat);
			constexpr size_t size = sizeof(XMFLOAT4X4);

			// FNV-1a 64bit
			size_t hash = 1469598103934665603ULL;
			for (std::size_t i = 0; i < size; ++i) {
				hash ^= data[i];
				hash *= 1099511628211ULL;
			}
			return hash;
		}

	};
} // namespace MG