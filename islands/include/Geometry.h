#pragma once

namespace islands {
namespace geometry {

struct AABB {
	glm::vec3 min, max;

	AABB transform(const glm::mat4& mat) const;
	bool contains(const AABB& aabb) const;
};

struct Triangle {
	glm::vec3 v0, v1, v2;

	bool isDegenerate() const;
	glm::vec3 getNormal() const;
	Triangle transform(const glm::mat4& mat) const;
	glm::vec3 getCenter() const;
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

struct CollisionMesh {
	std::vector<Triangle> triangles, collisionTriangles;
};

bool intersect(const AABB& a, const AABB& b);
bool intersect(const Triangle& triangle, const Sphere& sphere);
bool intersect(const Sphere& a, const Sphere& b);
bool intersect(const Sphere& sphere, const Plane& plane);
bool intersect(CollisionMesh& mesh, const Sphere& sphere);
float getSinking(const Triangle& triangle, const Sphere& sphere);
float getSinking(const Sphere& a, const Sphere& b);
float getSinking(const Sphere& sphere, const Plane& plane);

}
}