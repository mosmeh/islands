#include "Geometry.h"

namespace islands {
namespace geometry {

AABB AABB::transform(const glm::mat4& mat) const {
	AABB aabb{
		glm::vec3(INFINITY), glm::vec3(-INFINITY)
	};
	for (const auto& a : {min, max}) {
		for (const auto& b : {min, max}) {
			for (const auto& c : {min, max}) {
				const auto transformed = (mat * glm::vec4(a.x, b.y, c.z, 1)).xyz();
				aabb.min = glm::min(aabb.min, transformed);
				aabb.max = glm::max(aabb.max, transformed);
			}
		}
	}
	return aabb;
}

bool AABB::contains(const AABB& aabb) const {
	return glm::all(glm::lessThanEqual(min, aabb.min)) &&
		glm::all(glm::greaterThanEqual(max, aabb.max));
}

bool Triangle::isDegenerate() const {
	const auto a = glm::normalize(v1 - v0);
	const auto b = glm::normalize(v2 - v1);
	const auto c = glm::normalize(v0 - v2);

	static constexpr auto ALMOST_ONE = 1.f - glm::epsilon<float>();
	return (glm::abs(glm::dot(a, b)) >= ALMOST_ONE)
		|| (glm::abs(glm::dot(b, c)) >= ALMOST_ONE)
		|| (glm::abs(glm::dot(c, a)) >= ALMOST_ONE);
}

glm::vec3 Triangle::getNormal() const {
	const auto normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
	if (glm::any(glm::isnan(normal))) {
		return glm::zero<glm::vec3>();
	} else {
		return normal;
	}
}

Triangle Triangle::transform(const glm::mat4& mat) const {
	return Triangle{
		(mat * glm::vec4(v0, 1)).xyz(),
		(mat * glm::vec4(v1, 1)).xyz(),
		(mat * glm::vec4(v2, 1)).xyz()
	};
}

glm::vec3 Triangle::getCenter() const {
	return (v0 + v1 + v2) / 3.f;
}

bool intersect(const AABB& a, const AABB& b) {
	return glm::all(glm::greaterThanEqual(a.max, b.min)) &&
		glm::all(glm::lessThanEqual(a.min, b.max));
}

bool intersect(const Triangle& triangle, const Sphere& sphere) {
	assert(!triangle.isDegenerate());
	assert(sphere.radius >= 0.f);

	// see http://realtimecollisiondetection.net/blog/?p=103

	const auto a = triangle.v0 - sphere.center;
	const auto b = triangle.v1 - sphere.center;
	const auto c = triangle.v2 - sphere.center;
	const auto v = glm::cross(b - a, c - a);
	const auto d = glm::dot(a, v);
	/*if (glm::dot(v, sphere.center) < d) {
		return false;
	}*/
	const auto e = glm::dot(v, v);
	const auto rr = sphere.radius * sphere.radius;
	if (d * d > rr * e) {
		return false;
	}

	const auto aa = glm::dot(a, a);
	const auto ab = glm::dot(a, b);
	const auto ac = glm::dot(a, c);
	const auto bb = glm::dot(b, b);
	const auto bc = glm::dot(b, c);
	const auto cc = glm::dot(c, c);
	if (((aa > rr) && (ab > aa) && (ac > aa)) ||
		((bb > rr) && (ab > bb) && (bc > bb)) ||
		((cc > rr) && (ac > cc) && (bc > cc))) {
		return false;
	}

	const auto AB = b - a;
	const auto d1 = ab - aa;
	const auto e1 = glm::dot(AB, AB);
	const auto q1 = a * e1 - d1 * AB;
	const auto qc = c * e1 - q1;
	if ((glm::dot(q1, q1) > rr * e1 * e1) && (glm::dot(q1, qc) > 0)) {
		return false;
	}

	const auto BC = c - b;
	const auto d2 = bc - bb;
	const auto e2 = glm::dot(BC, BC);
	const auto q2 = b * e2 - d2 * BC;
	const auto qa = a * e2 - q2;
	if ((glm::dot(q2, q2) > rr * e2 * e2) && (glm::dot(q2, qa) > 0)) {
		return false;
	}

	const auto CA = a - c;
	const auto d3 = ac - cc;
	const auto e3 = glm::dot(CA, CA);
	const auto q3 = c * e3 - d3 * CA;
	const auto qb = b * e3 - q3;
	if ((glm::dot(q3, q3) > rr * e3 * e3) && (glm::dot(q3, qb) > 0)) {
		return false;
	}

	return true;
}

bool intersect(const Sphere& a, const Sphere& b) {
	assert(a.radius >= 0.f && b.radius >= 0.f);
	return glm::distance(a.center, b.center) <= a.radius + b.radius;
}

bool intersect(const Sphere& sphere, const Plane& plane) {
	assert(sphere.radius >= 0.f);
	const auto a = glm::length(plane.normal) * sphere.radius - plane.d;
	return glm::dot(plane.normal, sphere.center) <= a;
}

bool intersect(CollisionMesh& mesh, const Sphere& sphere) {
	bool intersected = false;
	mesh.collisionTriangles.clear();
	for (const auto& triangle : mesh.triangles) {
		if (intersect(triangle, sphere)) {
			intersected = true;

			mesh.collisionTriangles.emplace_back(triangle);
		}
	}
	
	return intersected;
}

float getSinking(const Triangle& triangle, const Sphere& sphere) {
	assert(!triangle.isDegenerate());
	assert(sphere.radius >= 0.f);
	const auto dist = glm::dot(triangle.getNormal(), sphere.center - triangle.getCenter());
	return sphere.radius - dist;
}

float getSinking(const Sphere& a, const Sphere& b) {
	assert(a.radius >= 0.f && b.radius >= 0.f);
	return a.radius + b.radius - glm::distance(a.center, b.center);
}

float getSinking(const Sphere& sphere, const Plane& plane) {
	assert(sphere.radius >= 0.f);
	const auto a = glm::length(plane.normal) * sphere.radius - plane.d;
	return a - glm::dot(plane.normal, sphere.center);
}

glm::quat directionToQuaternion(const glm::vec3& dir, const glm::vec3& front) {
	if (glm::dot(dir, -front) < 1.f - glm::epsilon<float>()) {
		return glm::rotation(front, dir);
	} else {
		return glm::angleAxis(glm::pi<float>(), glm::vec3(0, 0, 1.f));
	}
}

}
}
