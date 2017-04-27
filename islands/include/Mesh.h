#pragma once

#include "Resource.h"
#include "Material.h"

namespace islands {

class Collider;
class MeshCollider;
class SphereCollider;

class Mesh : public Resource {
	friend class Collider;
	friend class MeshCollider;
	friend class SphereCollider;

public:
	Mesh(const aiMesh* mesh, const aiMaterial* material);
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh(Mesh&& mesh) = default;

	virtual ~Mesh();

	bool isLoaded() const override;
	void draw();

	bool hasUV() const;
	std::shared_ptr<Material> getMaterial() const;

private:
	GLuint vertexArray_, vertexBuffer_, indexBuffer_, normalBuffer_, uvBuffer_;
	std::unique_ptr<glm::vec3[]> vertices_, normals_;
	std::unique_ptr<glm::vec2[]> uvs_;
	std::unique_ptr<unsigned int[]> indices_;
	const size_t numVertices_, numIndices_;
	const bool hasUV_;
	std::shared_ptr<Material> material_;

	void loadImpl() override;
	void uploadImpl() override;
};

}
