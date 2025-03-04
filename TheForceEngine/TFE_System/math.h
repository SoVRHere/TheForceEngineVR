#pragma once

#include "types.h"
#include <cmath>
#include <float.h>
#include <limits>
#include <optional>

constexpr Vec2f operator-(const Vec2f& v) { return { -v.x, -v.y }; }
constexpr Vec2f operator+(const Vec2f& a, const Vec2f& b) { return { a.x + b.x, a.y + b.y }; }
constexpr Vec2f operator-(const Vec2f& a, const Vec2f& b) { return { a.x - b.x, a.y - b.y }; }
constexpr Vec2f operator*(const Vec2f& a, const Vec2f& b) { return { a.x * b.x, a.y * b.y }; }
constexpr Vec2f operator*(const Vec2ui& a, const Vec2f& b) { return { a.x * b.x, a.y * b.y }; }
constexpr Vec2f operator*(float a, const Vec2f& b) { return { a * b.x, a * b.y }; }
constexpr Vec2f operator-(float a, const Vec2f& b) { return { a - b.x, a - b.y }; }
constexpr Vec2f operator-(const Vec2f& a, float b) { return { a.x - b, a.y - b }; }
constexpr Vec2f operator/(const Vec2f& a, float b) { return { a.x / b, a.y / b }; }

constexpr Vec3f operator-(const Vec3f& v) { return { -v.x, -v.y, -v.z }; }
constexpr Vec3f operator^(const Vec3f& a, const Vec3f& b) { return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x }; } // cross product
constexpr float operator|(const Vec3f& a, const Vec3f& b) { return a.x * b.x + a.y * b.y + a.z * b.z; } // dot product
constexpr Vec3f operator+(const Vec3f& a, const Vec3f& b) { return { a.x + b.x, a.y + b.y, a.z + b.z }; }
constexpr Vec3f operator-(const Vec3f& a, const Vec3f& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }
constexpr Vec3f operator*(const Vec3f& a, const Vec3f& b) { return { a.x * b.x, a.y * b.y, a.z * b.z }; }
constexpr Vec3f operator*(const Vec3f& a, float b) { return { a.x * b, a.y * b, a.z * b }; }
constexpr Vec3f operator*(float a, const Vec3f& b) { return { a * b.x, a * b.y, a * b.z }; }

constexpr Vec4f operator-(const Vec4f& v) { return { -v.x, -v.y, -v.z, -v.w }; }

constexpr bool operator==(const Vec2i& a, const Vec2i& b) { return a.x == b.x && a.y == b.y; }
constexpr bool operator!=(const Vec2i& a, const Vec2i& b) { return !operator==(a, b); }

constexpr bool operator==(const Vec2ui& a, const Vec2ui& b) { return a.x == b.x && a.y == b.y; }
constexpr bool operator!=(const Vec2ui& a, const Vec2ui& b) { return !operator==(a, b); }

namespace TFE_Math
{
	inline f32 sign(f32 x)
	{
		return x < 0.0f ? -1.0f : 1.0f;
	}

	inline bool isPow2(u32 x)
	{
		return (x > 0) && (x & (x - 1)) == 0;
	}

	inline bool isPow2(s32 x)
	{
		return (x > 0) && (x & (x - 1)) == 0;
	}

	inline u32 log2(u32 x)
	{
		if (x == 0 || x == 1) { return 0; }

		u32 l2 = 0;
		while (x > 1)
		{
			l2++;
			x >>= 1;
		}
		return l2;
	}

	inline u32 nextPow2(u32 x)
	{
		if (x == 0) { return 0; }

		x--;
		x |= x >> 1;
		x |= x >> 2;
		x |= x >> 4;
		x |= x >> 8;
		x |= x >> 16;
		x++;
		return x;
	}

	inline f32 fract(f32 x)
	{
		return x - floorf(x);
	}

	// the effective range is (-4.8, 4.8]. Outside of that the value is clamped at [-1, 1].
	inline f32 tanhf_series(f32 beta)
	{
		if (beta > 4.8f) { return 1.0f; }
		else if (beta <= -4.8f) { return -1.0f; }

		const f32 x2 = beta * beta;
		const f32 a  = beta * (135135.0f + x2 * (17325.0f + x2 * (378.0f + x2)));
		const f32 b  = 135135.0f + x2 * (62370.0f + x2 * (3150.0f + x2 * 28.0f));
		return a / b;
	}

	inline f32 dot(const Vec2f* p0, const Vec2f* p1)
	{
		return p0->x*p1->x + p0->z*p1->z;
	}

	inline f32 dot(const Vec2f& p0, const Vec2f& p1)
	{
		return p0.x * p1.x + p0.z * p1.z;
	}

	inline f32 dot(const Vec3f* p0, const Vec3f* p1)
	{
		return p0->x*p1->x + p0->y*p1->y + p0->z*p1->z;
	}

	inline f32 dot(const Vec3f& p0, const Vec3f& p1)
	{
		return p0.x * p1.x + p0.y * p1.y + p0.z * p1.z;
	}

	inline f32 dot(const Vec4f* p0, const Vec4f* p1)
	{
		return p0->x*p1->x + p0->y*p1->y + p0->z*p1->z + p0->w*p1->w;
	}

	inline f32 dot(const Vec4f& p0, const Vec4f& p1)
	{
		return p0.x * p1.x + p0.y * p1.y + p0.z * p1.z + p0.w * p1.w;
	}

	inline f32 distance(const Vec3f* p0, const Vec3f* p1)
	{
		Vec3f diff = { p1->x - p0->x, p1->y - p0->y, p1->z - p0->z };
		return sqrtf(dot(&diff, &diff));
	}

	inline f32 distance(const Vec2f* p0, const Vec2f* p1)
	{
		Vec2f diff = { p1->x - p0->x, p1->z - p0->z };
		return sqrtf(dot(&diff, &diff));
	}

	inline f32 distanceSq(const Vec3f* p0, const Vec3f* p1)
	{
		Vec3f diff = { p1->x - p0->x, p1->y - p0->y, p1->z - p0->z };
		return dot(&diff, &diff);
	}

	inline f32 distanceSq(const Vec2f* p0, const Vec2f* p1)
	{
		Vec2f diff = { p1->x - p0->x, p1->z - p0->z };
		return dot(&diff, &diff);
	}
		
	inline Vec3f normalize(const Vec3f* vec)
	{
		const f32 lenSq = dot(vec, vec);
		Vec3f nrm;
		if (lenSq > FLT_EPSILON)
		{
			const f32 scale = 1.0f / sqrtf(lenSq);
			nrm = { vec->x * scale, vec->y * scale, vec->z * scale };
		}
		else
		{
			nrm = *vec;
		}
		return nrm;
	}

	inline Vec3f normalize(const Vec3f& vec)
	{
		return normalize(&vec);
	}

	inline Vec2f normalize(const Vec2f* vec)
	{
		const f32 lenSq = dot(vec, vec);
		Vec2f nrm;
		if (lenSq > FLT_EPSILON)
		{
			const f32 scale = 1.0f / sqrtf(lenSq);
			nrm = { vec->x * scale, vec->z * scale };
		}
		else
		{
			nrm = *vec;
		}
		return nrm;
	}

	inline Vec2f normalize(const Vec2f& vec)
	{
		return normalize(&vec);
	}

	inline Vec3f cross(const Vec3f* a, const Vec3f* b)
	{
		return { a->y*b->z - a->z*b->y, a->z*b->x - a->x*b->z, a->x*b->y - a->y*b->x };
	}

	inline Vec3f cross(const Vec3f& a, const Vec3f& b)
	{
		return cross(&a, &b);
	}

	inline f32 planeDist(const Vec4f* plane, const Vec3f* pos)
	{
		return plane->x*pos->x + plane->y*pos->y + plane->z*pos->z + plane->w;
	}

	inline Vec3f add(const Vec3f& a, const Vec3f& b)
	{
		return { a.x + b.x, a.y + b.y, a.z + b.z };
	}

	inline Vec3f sub(const Vec3f& a, const Vec3f& b)
	{
		return { a.x - b.x, a.y - b.y, a.z - b.z };
	}

	inline Vec3f scale(const Vec3f& a, float scale)
	{
		return { scale * a.x, scale * a.y, scale * a.z };
	}

	inline f32 length(const Vec2f& v)
	{
		return std::sqrtf(dot(v, v));
	}

	inline f32 length(const Vec3f& v)
	{
		return std::sqrtf(dot(v, v));
	}

	template<typename T>
	constexpr T radians(T degrees) noexcept
	{
		static_assert(std::numeric_limits<T>::is_iec559, "'radians' only accept floating-point input");
		return degrees * static_cast<T>(0.01745329251994329576923690768489);
	}

	template<typename T>
	constexpr T degrees(T radians) noexcept
	{
		static_assert(std::numeric_limits<T>::is_iec559, "'degrees' only accept floating-point input");
		return radians * static_cast<T>(57.295779513082320876798154814105);
	}

	Mat3 computeViewMatrix(const Vec3f* lookDir, const Vec3f* upDir);
	Mat3 transpose(const Mat3& mtx);
	Mat4 transpose4(Mat4 mtx);
	Mat4 computeProjMatrix(f32 fovInRadians, f32 aspectRatio, f32 zNear, f32 zFar);
	Mat4 computeProjMatrixExplicit(f32 xScale, f32 yScale, f32 zNear, f32 zFar);
	Mat4 computeInvProjMatrix(const Mat4& mtx);
	Mat4 mulMatrix4(const Mat4& mtx0, const Mat4& mtx1);
	Vec4f multiply(const Mat4& mtx, const Vec4f& vec);
	Mat4 buildMatrix4(const Mat3& mtx, const Vec3f& pos);
	Mat3 getMatrix3(const Mat4& mtx);
	Vec3f getVec3(const Vec4f& v);
	Vec3f getTranslation(const Mat4& mtx);
	const Mat4& getIdentityMatrix4();
	const Mat3& getIdentityMatrix3();

	void buildRotationMatrix(Vec3f angles, Vec3f* mat);

	bool lineSegmentIntersect(const Vec2f* a0, const Vec2f* a1, const Vec2f* b0, const Vec2f* b1, f32* s, f32* t);
	bool lineYPlaneIntersect(const Vec3f* p0, const Vec3f* p1, f32 planeHeight, Vec3f* hitPoint);

	// Closest point between two lines (p0, p1) and (p2, p3).
	// Returns false if no closest point can be found.
	bool closestPointBetweenLines(const Vec3f* p1, const Vec3f* p2, const Vec3f* p3, const Vec3f* p4, f32* u, f32* v);

	inline float getLinePointDistance(
		const Vec3f& linePoint0, const Vec3f& linePoint1,
		const Vec3f& point)
	{
		return length((linePoint1 - linePoint0) ^ (linePoint0 - point)) / length(linePoint1 - linePoint0);
	}

	inline Vec3f getClosestPointOnLine(const Vec3f& linePoint0, const Vec3f& linePoint1, const Vec3f& point)
	{
		const Vec3f ab = linePoint1 - linePoint0;
		const Vec3f ap = point - linePoint0;
		float t = (ap | ab) / (ab | ab);
		return linePoint0 + t * ab;
	}

	struct RayPlaneIntersection
	{
		Vec3f point;
		float t;
	};
	inline std::optional<RayPlaneIntersection> getRayPlaneIntersection(
		const Vec3f& rayPoint, const Vec3f& rayVector,
		const Vec3f& planeNormal, float d)			// plane: ax + by + cz + d = 0, planeNormal = [a, b, c]
	{
		const float denom = dot(planeNormal, rayVector);
		if (std::abs(denom) <= 1e-4f)
			return std::nullopt; // ~ parallel

		const float t = -((planeNormal | rayPoint) + d) / denom;

		return RayPlaneIntersection{ rayPoint + t * rayVector, t };
	}

	inline std::optional<RayPlaneIntersection> getRayPlaneIntersection(
		const Vec3f& rayPoint, const Vec3f& rayVector,
		const Vec3f& planePoint, const Vec3f& planeNormal)
	{
		const float denom = planeNormal | rayVector;
		if (std::abs(denom) <= 1e-4f)
			return std::nullopt; // ~ parallel

		const float t = ((planePoint | planeNormal) - (planeNormal | rayPoint)) / denom;

		return RayPlaneIntersection{ rayPoint + t * rayVector, t };
	}
}
