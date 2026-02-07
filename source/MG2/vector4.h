#pragma once

#include <DirectXMath.h>
#include <string>
#include "vector3.h"

using namespace DirectX;

namespace MG {
    struct Vector4;
    typedef Vector4 RGBA;
    typedef Vector4 Quaternion;
    struct Vector4
    {
    private:
        static constexpr const float PI = 3.14159274f;

    public:
        union {
            struct { float r, g, b, a; };
            struct { float x, y, z, w; };
            float data[4];
        };

        // コンストラクタ
        Vector4(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f, float _w = 0.0f) : x(_x), y(_y), z(_z), w(_w) {}
        Vector4(const Vector3& v) : x(v.x), y(v.y), z(v.z), w(0.0f) {}
        Vector4(const XMFLOAT4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
        Vector4(const XMVECTOR& v) { XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), v); }
        Vector4(const Vector4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
        Vector4(const char* hexStr) {
            unsigned long hex = std::stoul(hexStr, nullptr, 16);
            float over255 = 1.0f / 255.0f;
            if (hex > 0xFFFFFF) {
                x = (float)(hex >> 24 & 0xFF) * over255;
                y = (float)(hex >> 16 & 0xFF) * over255;
                z = (float)(hex >> 8 & 0xFF) * over255;
                w = (float)(hex & 0xFF) * over255;
            }
            else if (hex > 0xFFF) {
                x = (float)(hex >> 16 & 0xFF) * over255;
                y = (float)(hex >> 8 & 0xFF) * over255;
                z = (float)(hex & 0xFF) * over255;
                w = 1.0f;
            }
            else {
                x = (float)(((hex >> 8 & 0xF) << 4) | (hex >> 8 & 0xF));
                y = (float)(((hex >> 4 & 0xF) << 4) | (hex >> 4 & 0xF));
                z = (float)(((hex & 0xF) << 4) | (hex & 0xF));
                w = 1.0f;
            }
        }

        // 暗黙変換
        operator Vector3() const { return Vector3{ x,y,z }; }
        operator XMFLOAT4() const { return XMFLOAT4{ x,y,z,w }; }
        operator XMVECTOR() const { return XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(this)); }

        bool operator==(const Vector4& other) const {
            return
                x == other.x &&
                y == other.y &&
                z == other.z &&
                w == other.w;
        }

        // 代入
        Vector4& operator=(const Vector4& v) { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }
        Vector4& operator=(const XMFLOAT4& v) { x = v.x; y = v.y; z = v.z; w = v.w; return *this; }
        Vector4& operator=(const XMVECTOR& v) { XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), v); return *this; }

        // 加算・減算
        Vector4 operator+(const Vector4& v) const { return { x + v.x, y + v.y, z + v.z, w + v.w }; }
        Vector4 operator+(const XMFLOAT4& v) const { return { x + v.x, y + v.y, z + v.z,  w + v.w }; }
        XMVECTOR operator+(const XMVECTOR& v) const { return XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(this)) + v; }

        Vector4& operator+=(const Vector4& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
        Vector4& operator+=(const XMFLOAT4& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
        Vector4& operator+=(const XMVECTOR& v) { 
            XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(this)) + v);
            return *this; 
        }

        Vector4 operator-(const Vector4& v) const { return { x - v.x, y - v.y, z - v.z, w - v.w }; }
        Vector4 operator-(const XMFLOAT4& v) const { return { x - v.x, y - v.y, z - v.z, w - v.w }; }
        XMVECTOR operator-(const XMVECTOR& v) const { return XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(this)) - v; }

        Vector4& operator-=(const Vector4& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
        Vector4& operator-=(const XMFLOAT4& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
        Vector4& operator-=(const XMVECTOR& v) { 
            XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(this)) - v);
            return *this;
        }

        // スカラー
        Vector4 operator*(float s) const { return { x * s, y * s, z * s, w * s }; }
        Vector4 operator/(float s) const { return { x / s, y / s, z / s, w / s }; }
        Vector4& operator*=(float s) { x *= s; y *= s; z *= s; w *= s; return *this; }
        Vector4& operator/=(float s) { x /= s; y /= s; z /= s; w /= s; return *this; }

        bool operator ==(const Vector4& v) { return x == v.x && y == v.y && z == v.z && w == v.w; }
        bool operator !=(const Vector4& v) { return x != v.x || y != v.y || z != v.z || w != v.w; }

        float Length() const
        {
            XMVECTOR v = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(this));
            return XMVectorGetX(XMVector4Length(v));
        }

        float LengthSq() const
        {
            XMVECTOR v = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(this));
            return XMVectorGetX(XMVector4LengthSq(v));
        }

        void Normalize() {
            XMVECTOR v = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(this));
            *this = XMVector4Normalize(v);
        }

        static XMVECTOR Normalize(const Vector4& value) {
            return XMVector4Normalize(XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&value)));
        }

        // Hermite 補間
        static XMVECTOR Hermite(const Vector4& p0, const Vector4& v0, const Vector4& p1, const Vector4& v1, float t) {
            float t2 = t * t;
            float t3 = t2 * t;

            float h1 = 2.0f * t3 - 3.0f * t2 + 1.0f; // for p0
            float h2 = -2.0f * t3 + 3.0f * t2;        // for p1
            float h3 = t3 - 2.0f * t2 + t;     // for v0
            float h4 = t3 - t2;          // for v1

            XMVECTOR P0 = p0;
            XMVECTOR P1 = p1;
            XMVECTOR V0 = v0;
            XMVECTOR V1 = v1;

            XMVECTOR result = XMVectorMultiply(P0, XMVectorReplicate(h1));
            result = XMVectorMultiplyAdd(P1, XMVectorReplicate(h2), result);
            result = XMVectorMultiplyAdd(V0, XMVectorReplicate(h3), result);
            result = XMVectorMultiplyAdd(V1, XMVectorReplicate(h4), result);

            return result;
        }

        // 三次ベジエ
        static XMVECTOR Bezier3(const Vector4& p0, const Vector4& p1, const Vector4& p2, const Vector4& p3, float t) {
            float u = 1.0f - t;
            float u2 = u * u;
            float u3 = u2 * u;
            float t2 = t * t;
            float t3 = t2 * t;

            float b0 = u3;          // (1-t)^3
            float b1 = 3.0f * u2 * t; // 3(1-t)^2 t
            float b2 = 3.0f * u * t2; // 3(1-t) t^2
            float b3 = t3;            // t^3

            XMVECTOR P0 = p0;
            XMVECTOR P1 = p1;
            XMVECTOR P2 = p2;
            XMVECTOR P3 = p3;

            XMVECTOR result = XMVectorMultiply(P0, XMVectorReplicate(b0));
            result = XMVectorMultiplyAdd(P1, XMVectorReplicate(b1), result);
            result = XMVectorMultiplyAdd(P2, XMVectorReplicate(b2), result);
            result = XMVectorMultiplyAdd(P3, XMVectorReplicate(b3), result);

            return result;
        }

        static XMVECTOR AxisRadian(const Vector3& axis, float radian) {
            return XMQuaternionRotationAxis(axis, radian);
        }

        static XMVECTOR AxisRadian(const XMVECTOR& axis, float radian) {
            return XMQuaternionRotationAxis(axis, radian);
        }

        static XMVECTOR AxisDegree(const Vector3& axis, float degree) {
            return XMQuaternionRotationAxis(axis, XMConvertToRadians(degree));
        }

        static XMVECTOR AxisDegree(const XMVECTOR& axis, float degree) {
            return XMQuaternionRotationAxis(axis, XMConvertToRadians(degree));
        }

        static XMVECTOR MakeQuaternion(const float pitch, const float yaw, const float roll) {
            return XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
        }

        static XMVECTOR MakeQuaternion(const Vector3& forward, const Vector3& upper)
        {

            XMVECTOR f = XMVector3Normalize(forward);
            XMVECTOR r = XMVector3Normalize(XMVector3Cross(upper, f));
            XMVECTOR u = XMVector3Cross(f, r);

            XMMATRIX m(
                r,
                u,
                f,
                XMVectorSet(0.f, 0.f, 0.f, 1.f)
            );

            //m = XMMatrixTranspose(m);
            return XMQuaternionRotationMatrix(m);
        }

        static XMVECTOR Slerp(const Quaternion& a, const Quaternion& b, const float t) {
            return XMQuaternionSlerp(a, b, t);
        }

        static const Vector4 ZERO;
        static const Vector4 ONE;
        static const RGBA BLACK;
        static const RGBA WHITE;
        static const Quaternion IDENTITY;

        static RGBA HSVtoRGBA(float h, float s, float v, float a)
        {
            float r = v;
            float g = v;
            float b = v;

            if (s > 0.0f)
            {
                h = fmodf(h, 1.0f);
                if (h < 0.0f) h += 1.0f;

                h *= 6.0f;
                int i = (int)h;
                float f = h - (float)i;

                switch (i)
                {
                case 0: g *= 1 - s * (1 - f); b *= 1 - s;             break;
                case 1: r *= 1 - s * f;       b *= 1 - s;             break;
                case 2: r *= 1 - s;           b *= 1 - s * (1 - f);   break;
                case 3: r *= 1 - s;           g *= 1 - s * f;         break;
                case 4: r *= 1 - s * (1 - f); g *= 1 - s;             break;
                case 5: g *= 1 - s;           b *= 1 - s * f;         break;
                default: break;
                }
            }

            return { r, g, b, a };
        }

        static Vector4 GetRandom();

        static Vector4 GetSignedRandom();

        static Vector4 Max(const Vector4& a, const Vector4& b);
        static Vector4 Min(const Vector4& a, const Vector4& b);
    };

    
    

} // namespace MG