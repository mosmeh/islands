#include "Geometry.h"

namespace islands {
namespace geometry {

bool intersect(const AABB& a, const AABB& b) {
	return glm::all(glm::greaterThan(a.max, b.min)) &&
		glm::all(glm::lessThan(a.min, b.max));
}

bool intersect(const Triangle& triangle, const Sphere& sphere) {
	assert(sphere.radius > 0);

	// see http://realtimecollisiondetection.net/blog/?p=103

	const auto a = triangle.v0 - sphere.center;
	const auto b = triangle.v1 - sphere.center;
	const auto c = triangle.v2 - sphere.center;
	const auto v = glm::cross(b - a, c - a);
	const auto d = glm::dot(a, v);
	if (glm::dot(v, sphere.center) < d) {
		return false;
	}
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
	if (((aa > rr) & (ab > aa) & (ac > aa)) ||
		((bb > rr) & (ab > bb) & (bc > bb)) ||
		((cc > rr) & (ac > cc) & (bc > cc))) {
		return false;
	}

	const auto AB = b - a;
	const auto BC = c - b;
	const auto CA = a - c;
	const auto d1 = ab - aa;
	const auto d2 = bc - bb;
	const auto d3 = ac - cc;
	const auto e1 = glm::dot(AB, AB);
	const auto e2 = glm::dot(BC, BC);
	const auto e3 = glm::dot(CA, CA);
	const auto q1 = a * e1 - d1 * AB;
	const auto q2 = b * e2 - d2 * BC;
	const auto q3 = c * e3 - d3 * CA;
	const auto qc = c * e1 - q1;
	const auto qa = a * e2 - q2;
	const auto qb = b * e3 - q3;

	return !(((glm::dot(q1, q1) > rr * e1 * e1) & (glm::dot(q1, qc) > 0)) ||
		((glm::dot(q2, q2) > rr * e2 * e2) & (glm::dot(q2, qa) > 0)) ||
		((glm::dot(q3, q3) > rr * e3 * e3) & (glm::dot(q3, qb) > 0)));
}

bool intersect(const Sphere& a, const Sphere& b) {
	assert(a.radius > 0 && b.radius > 0);
	return glm::distance(a.center, b.center) <= a.radius + b.radius;
}

float getSinking(const Sphere & a, const Sphere & b) {
	assert(a.radius > 0 && b.radius > 0);
	return a.radius + b.radius - glm::distance(a.center, b.center);
}

bool intersect(const Sphere& sphere, const Plane& plane) {
	assert(sphere.radius > 0);
	const auto a = glm::length(plane.normal) * sphere.radius - plane.d;
	return glm::dot(plane.normal, sphere.center) <= a;
}

float getSinking(const Sphere & sphere, const Plane & plane) {
	assert(sphere.radius > 0);
	const auto a = glm::length(plane.normal) * sphere.radius - plane.d;
	return a - glm::dot(plane.normal, sphere.center);
}

glm::vec3 Triangle::getNormal() const {
	return glm::normalize(glm::cross(v1 - v0, v2 - v0));
}

}
}