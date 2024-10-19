#pragma once

#define PI 3.14159265358979323846
#include <cmath>
#include <physx/foundation/PxVec3.h>
#include <physx/foundation/PxQuat.h>

constexpr float DegToRad = PI / 180.f;
constexpr float RadToDeg = 180.f / PI;

inline float Clamp(float n, float lower, float upper)
{
	return std::max<float>(lower, std::min<float>(n, upper));
}

inline float MoveTowards(float current, float target, float maxDelta)
{
	if (std::abs(target - current) <= maxDelta) {
		return target;
	}

	return current + std::copysign(maxDelta, target - current);
}

inline physx::PxQuat AngleAxis(float degrees, physx::PxVec3 axis)
{
	float radians = degrees * DegToRad;
	radians *= 0.5f;

	float sina, cosa;
	sina = std::sin(radians);
	cosa = std::cos(radians);

	axis *= sina;

	return physx::PxQuat(axis.x, axis.y, axis.z, cosa);
}