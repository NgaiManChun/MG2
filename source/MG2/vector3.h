#pragma once
#include <DirectXMath.h>
using namespace DirectX;

namespace MG {
    struct Vector3
    {
        float x, y, z;

        Vector3(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f) : x(_x), y(_y), z(_z) {}
        Vector3(const XMFLOAT3& v) : x(v.x), y(v.y), z(v.z) {}
        Vector3(const XMVECTOR& v) { XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), v); }
        Vector3(const Vector3& v) : x(v.x), y(v.y), z(v.z) {}

        operator XMFLOAT3() const { return XMFLOAT3{ x,y,z }; }
        operator XMVECTOR() const { return XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(this)); }

        Vector3& operator=(const Vector3& v) { x = v.x; y = v.y; z = v.z; return *this; }
        Vector3& operator=(const XMFLOAT3& v) { x = v.x; y = v.y; z = v.z; return *this; }
        Vector3& operator=(const XMVECTOR& v) { XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), v); return *this; }

        Vector3 operator+(const Vector3& v) const { return { x + v.x, y + v.y, z + v.z }; }
        Vector3 operator+(const XMFLOAT3& v) const { return { x + v.x, y + v.y, z + v.z }; }
        XMVECTOR operator+(const XMVECTOR& v) const { return XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(this)) + v; }

        Vector3& operator+=(const Vector3& v) { x += v.x; y += v.y; z += v.z; return *this; }
        Vector3& operator+=(const XMFLOAT3& v) { x += v.x; y += v.y; z += v.z; return *this; }
        Vector3& operator+=(const XMVECTOR& v) 
        { 
            XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(this)) + v);
            return *this;
        }

        Vector3 operator-(const Vector3& v) const { return { x - v.x, y - v.y, z - v.z }; }
        Vector3 operator-(const XMFLOAT3& v) const { return { x - v.x, y - v.y, z - v.z }; }
        XMVECTOR operator-(const XMVECTOR& v) const { return XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(this)) - v; }

        Vector3& operator-=(const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
        Vector3& operator-=(const XMFLOAT3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
        Vector3& operator-=(const XMVECTOR& v) 
        { 
            XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), XMLoadFloat3(reinterpret_cast<XMFLOAT3*>(this)) - v);
            return *this;
        }

        Vector3 operator*(float s) const { return { x * s, y * s, z * s }; }
        Vector3 operator/(float s) const { return { x / s, y / s, z / s }; }
        Vector3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
        Vector3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }

        bool operator ==(const Vector3& v) { return x == v.x && y == v.y && z == v.z; }
        bool operator !=(const Vector3& v) { return x != v.x || y != v.y || z != v.z; }

        float Length() const
        {
            XMVECTOR v = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(this));
            return XMVectorGetX(XMVector3Length(v));
        }

        float LengthSq() const
        {
            XMVECTOR v = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(this));
            return XMVectorGetX(XMVector3LengthSq(v));
        }

        void Normalize() 
        {
            XMVECTOR v = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(this));
            *this = XMVector3Normalize(v);
        }

        static XMVECTOR Normalize(const Vector3& value) {
            return XMVector3Normalize(XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&value)));
        }

        static XMVECTOR Normalize(const XMVECTOR& value) {
            return XMVector3Normalize(value);
        }

        float Dot(const Vector3& v) const 
        {
            XMVECTOR a = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(this));
            XMVECTOR b = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v));
            return XMVectorGetX(XMVector3Dot(a, b));
        }

        float Dot(const XMFLOAT3& v) const 
        {
            XMVECTOR a = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(this));
            XMVECTOR b = XMLoadFloat3(&v);
            return XMVectorGetX(XMVector3Dot(a, b));
        }

        float Dot(const XMVECTOR& v) const 
        {
            XMVECTOR a = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(this));
            return XMVectorGetX(XMVector3Dot(a, v));
        }

        static float Dot(const Vector3& a, const Vector3& b) 
        {
            return a.Dot(b);
        }

        XMVECTOR Cross(const Vector3& v) const 
        {
            return XMVector3Cross(
                XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(this)), 
                XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&v)));
        }

        XMVECTOR Cross(const XMFLOAT3& v) const 
        {
            return XMVector3Cross(XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(this)), XMLoadFloat3(&v));
        }

        XMVECTOR Cross(const XMVECTOR& v) const 
        {
            return XMVector3Cross(XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(this)), v);
        }

        static XMVECTOR Cross(const Vector3& a, const Vector3& b) 
        {
            return a.Cross(b);
        }

        static XMVECTOR Lerp(const Vector3& a, const Vector3& b, const float t) 
        {
            return XMVectorLerp(a, b, t);
        }

        // Hermite 補間
        static XMVECTOR Hermite(const Vector3& p0, const Vector3& v0, const Vector3& p1, const Vector3& v1, float t) 
        {

            float t2 = t * t;
            float t3 = t2 * t;

            float h1 = 2.0f * t3 - 3.0f * t2 + 1.0f;    // p0
            float h2 = -2.0f * t3 + 3.0f * t2;          // p1
            float h3 = t3 - 2.0f * t2 + t;              // v0
            float h4 = t3 - t2;                         // v1

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
        static XMVECTOR Bezier3(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& p3, float t) 
        {
            float u = 1.0f - t;
            float u2 = u * u;
            float u3 = u2 * u;
            float t2 = t * t;
            float t3 = t2 * t;

            float b0 = u3;              // (1-t)^3
            float b1 = 3.0f * u2 * t;   // 3(1-t)^2 t
            float b2 = 3.0f * u * t2;   // 3(1-t) t^2
            float b3 = t3;              // t^3

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

        static const Vector3 ZERO;
        static const Vector3 ONE;

        static Vector3 GetRandom();

        static Vector3 GetSignedRandom();

        static Vector3 Max(const Vector3& a, const Vector3& b);

        static Vector3 Min(const Vector3& a, const Vector3& b);

    };
} // namespace MG