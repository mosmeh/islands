#pragma once

namespace islands {
namespace geometry {

struct AABB {
	glm::vec3 min, max;
};

struct Triangle {
	glm::vec3 v0, v1, v2;

	glm::vec3 getNormal() const;
};

struct Sphere {
	glm::vec3 center;
	float radius;
};

struct Plane {
	glm::vec3 normal;
	float d;
};

struct Capsule {
	float r;
	glm::vec3 a, b;
};

bool intersect(const AABB& a, const AABB& b);
bool intersect(const Triangle& triangle, const Sphere& sphere);
bool intersect(const Sphere& a, const Sphere& b);
float getSinking(const Sphere& a, const Sphere& b);
bool intersect(const Sphere& sphere, const Plane& plane);
float getSinking(const Sphere& sphere, const Plane& plane);

}
}