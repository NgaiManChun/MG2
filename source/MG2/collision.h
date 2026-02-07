#pragma once
#include "dataType.h"

namespace MG {

	enum COLLISION_TYPE {
		COLLISION_TYPE_BOX,
		COLLISION_TYPE_SPHERE,
		COLLISION_TYPE_CAPSULE
	};

	class CollisionUnit
	{
	private:
		COLLISION_TYPE m_Type;
		Matrix4x4 m_Transform;
		Matrix4x4 m_InvTransform;

	public:
		bool Overlap(CollisionUnit other)
		{
			switch (m_Type)
			{
			case MG::COLLISION_TYPE_BOX:
				other.m_Transform * m_InvTransform;



				

				break;
			case MG::COLLISION_TYPE_SPHERE:
				break;
			case MG::COLLISION_TYPE_CAPSULE:
				break;
			default:
				break;
			}
		}

	};

	


	//namespace Collision {

	//	class CollisionUnit;

	//	enum COLLISION_TYPE {
	//		COLLISION_TYPE_POINT,
	//		COLLISION_TYPE_AABB,
	//		COLLISION_TYPE_SPHERE,
	//		COLLISION_TYPE_BOX,
	//		COLLISION_TYPE_CAPSULE
	//	};

	//	struct COLLISION_UNIT {
	//		COLLISION_TYPE type;
	//		Matrix4x4 transform;
	//	};

	//	struct COLLISION_PAIR {
	//		const CollisionUnit* a;
	//		const CollisionUnit* b;
	//	};

	//	typedef CollisionUnit* (*DYNAMIC_COLLSION)();

	//	class Point;
	//	class AABB;
	//	class Sphere;
	//	class Box;
	//	class Capsule;
	//	class CollisionUnit {
	//	protected:
	//		CollisionUnit(const Matrix4x4& transform = Matrix4x4()) : transform(transform) {}
	//	public:
	//		Matrix4x4 transform = Matrix4x4::TranslatingMatrix({});

	//		virtual void Translation(const Vector3& move) = 0;
	//		virtual void Scale(const Vector3& scale) = 0;
	//		virtual void Rotate(const Quaternion& rotate) = 0;
	//		virtual void Transform(const Matrix4x4& matrix) = 0;
	//		virtual bool Overlap(const CollisionUnit* unit) const;
	//		virtual bool Overlap(const Vector3& p) const = 0;
	//		virtual bool Overlap(const Point& p) const = 0;
	//		virtual bool Overlap(const AABB& aabb) const = 0;
	//		virtual bool Overlap(const Sphere& sphere) const = 0;
	//		virtual bool Overlap(const Box& box) const = 0;
	//		virtual bool Overlap(const Capsule& capsule) const = 0;

	//		static CollisionUnit* Create(COLLISION_TYPE type, const Matrix4x4& transform)
	//		{
	//			if (type == COLLISION_TYPE_POINT) {
	//				return (CollisionUnit*) new Point(transform);
	//			}
	//			else if (type == COLLISION_TYPE_AABB) {
	//				return (CollisionUnit*) new AABB(transform);
	//			}
	//			else if (type == COLLISION_TYPE_SPHERE) {
	//				return (CollisionUnit*) new Sphere(transform);
	//			}
	//			else if (type == COLLISION_TYPE_BOX) {
	//				return (CollisionUnit*) new Box(transform);
	//			}
	//			else if (type == COLLISION_TYPE_CAPSULE) {
	//				return (CollisionUnit*) new Capsule(transform);
	//			}
	//			return (CollisionUnit*) new Box(transform);
	//		}
	//		static CollisionUnit Create(CollisionUnit* collisionUnit);

	//	};

	//	class Point : public CollisionUnit {
	//	public:
	//		Vector3 position;
	//		Point(const Vector3& position = {}, const std::string& name = "");
	//		Point(const Matrix4x4& transform, const std::string& name = "");
	//		void Translation(const Vector3& move) override;
	//		void Scale(const Vector3& scale) override;
	//		void Rotate(const Quaternion& rotate) override;
	//		void Transform(const Matrix4x4& matrix) override;
	//		bool Overlap(const Vector3& p) const override;
	//		bool Overlap(const Point& p) const override;
	//		bool Overlap(const AABB& aabb) const override;
	//		bool Overlap(const Sphere& sphere) const override;
	//		bool Overlap(const Box& box) const override;
	//		bool Overlap(const Capsule& capsule) const override;

	//	};

	//	class AABB : public CollisionUnit {
	//	public:
	//		Vector3 center;
	//		Vector3 halfSize;
	//		AABB(const Vector3& center = {}, const Vector3& halfSize = {}, const std::string& name = "");
	//		AABB(const Matrix4x4& transform, const std::string& name = "");
	//		void Translation(const Vector3& move) override;
	//		void Scale(const Vector3& scale) override;
	//		void Rotate(const Quaternion& rotate) override;
	//		void Transform(const Matrix4x4& matrix) override;
	//		bool Overlap(const Vector3& p) const override;
	//		bool Overlap(const Point& p) const override;
	//		bool Overlap(const AABB& aabb) const override;
	//		bool Overlap(const Sphere& sphere) const override;
	//		bool Overlap(const Box& box) const override;
	//		bool Overlap(const Capsule& capsule) const override;
	//	};

	//	class Sphere : public CollisionUnit {
	//	public:
	//		//Vector3 center;
	//		//Matrix4x4 transform = Matrix4x4::TranslatingMatrix({});
	//		Matrix4x4 invTransform = transform;
	//		Sphere(const Vector3& position = {}, const Vector3& scale = { 1.0f, 1.0f, 1.0f }, const Quaternion& rotate = Quaternion::IDENTITY, const std::string& name = "");
	//		Sphere(const Matrix4x4& transform, const std::string& name = "");
	//		void Translation(const Vector3& move) override;
	//		void Scale(const Vector3& scale) override;
	//		void Rotate(const Quaternion& rotate) override;
	//		void Transform(const Matrix4x4& matrix) override;
	//		bool Overlap(const Vector3& p) const override;
	//		bool Overlap(const Point& p) const override;
	//		bool Overlap(const AABB& aabb) const override;
	//		bool Overlap(const Sphere& sphere) const override;
	//		bool Overlap(const Box& box) const override;
	//		bool Overlap(const Capsule& capsule) const override;
	//	};

	//	struct FACE {
	//		Vector3* points[4];
	//		Vector3 normal;
	//	};

	//	class Box : public CollisionUnit {
	//	public:
	//		Vector3 center = {};
	//		Vector3 points[8] = {
	//			{  0.5f,	 0.5f,  0.5f }, // 右上奥	0
	//			{  0.5f,	 0.5f, -0.5f }, // 右上手前 1
	//			{  0.5f,	-0.5f,  0.5f }, // 右下奥	2 
	//			{  0.5f,	-0.5f, -0.5f }, // 右下手前 3
	//			{ -0.5f,	 0.5f,  0.5f }, // 左上奥	4
	//			{ -0.5f,	 0.5f, -0.5f }, // 左上手前 5
	//			{ -0.5f,	-0.5f,  0.5f }, // 左下奥	6
	//			{ -0.5f,	-0.5f, -0.5f }, // 左下手前 7
	//		};
	//		FACE faces[6] = {
	//			{ { points + 5, points + 1, points + 3, points + 7 }, {  0.0f,  0.0f, -1.0f } }, // 前 0
	//			{ { points + 1, points + 0, points + 2, points + 3 }, {  1.0f,  0.0f,  0.0f } }, // 右 1
	//			{ { points + 0, points + 4, points + 6, points + 2 }, {  0.0f,  0.0f,  1.0f } }, // 後 2
	//			{ { points + 4, points + 5, points + 7, points + 6 }, { -1.0f,  0.0f,  0.0f } }, // 左 3
	//			{ { points + 4, points + 0, points + 1, points + 5 }, {  0.0f,  1.0f,  0.0f } }, // 上 4
	//			{ { points + 3, points + 7, points + 6, points + 2 }, {  0.0f, -1.0f,  0.0f } }, // 下 5
	//		};
	//		FACE* relatedFaces[8][3] = {
	//			{ faces + 1, faces + 4, faces + 2 }, // 右上奥	
	//			{ faces + 1, faces + 4, faces + 0 }, // 右上手前
	//			{ faces + 1, faces + 5, faces + 2 }, // 右下奥	
	//			{ faces + 1, faces + 5, faces + 0 }, // 右下手前
	//			{ faces + 3, faces + 4, faces + 2 }, // 左上奥	
	//			{ faces + 3, faces + 4, faces + 0 }, // 左上手前
	//			{ faces + 3, faces + 5, faces + 2 }, // 左下奥	
	//			{ faces + 3, faces + 5, faces + 0 }, // 左下手前
	//		};
	//		//Matrix4x4 transform = Matrix4x4::TranslatingMatrix({});

	//		Box(const Vector3& position = {}, const Vector3& scale = { 1.0f, 1.0f, 1.0f }, const Quaternion& rotate = Quaternion::IDENTITY, const std::string& name = "");
	//		Box(const Matrix4x4& transform, const std::string& name = "");
	//		Box(const Box& box);
	//		void Translation(const Vector3& move) override;
	//		void Scale(const Vector3& scale) override;
	//		void Rotate(const Quaternion& rotate) override;
	//		void Transform(const Matrix4x4& matrix) override;
	//		bool Overlap(const Vector3& p) const override;
	//		bool Overlap(const Sphere& sphere) const override;
	//		bool Overlap(const Point& p) const override;
	//		bool Overlap(const AABB& aabb) const override;
	//		bool Overlap(const Box& box) const override;
	//		bool Overlap(const Capsule& capsule) const override;
	//		Vector3 GetNormal(const Vector3& position);
	//		void operator =(const Box& src);
	//	};

	//	class Capsule : public CollisionUnit {
	//	public:
	//		/*Vector3 center = {};
	//		float height;
	//		float radus;*/
	//		Matrix4x4 invTransform = transform;
	//		Capsule(const Vector3& position = {}, const Vector3& scale = { 1.0f, 1.0f, 1.0f }, const Quaternion& rotate = Quaternion::IDENTITY, const std::string& name = "");
	//		Capsule(const Matrix4x4& transform, const std::string& name = "");
	//		void Translation(const Vector3& move) override;
	//		void Scale(const Vector3& scale) override;
	//		void Rotate(const Quaternion& rotate) override;
	//		void Transform(const Matrix4x4& matrix) override;
	//		bool Overlap(const Vector3& p) const override;
	//		bool Overlap(const Sphere& sphere) const override;
	//		bool Overlap(const Point& p) const override;
	//		bool Overlap(const AABB& aabb) const override;
	//		bool Overlap(const Box& box) const override;
	//		bool Overlap(const Capsule& capsule) const override;
	//	};


	//} // namespace Collision
} // namespace MG