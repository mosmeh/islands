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

protected:
	GLuint vertexArray_;
	const size_t numVertices_;

	void uploadImpl() override;

private:
	GLuint vertexBuffer_, indexBuffer_, normalBuffer_, uvBuffer_;
	std::unique_ptr<glm::vec3[]> vertices_, normals_;
	std::unique_ptr<glm::vec2[]> uvs_;
	std::unique_ptr<unsigned int[]> indices_;
	const size_t numIndices_;
	const bool hasUV_;
	std::shared_ptr<Material> material_;

	void loadImpl() override;
};

constexpr size_t NUM_BONES_PER_VERTEX = 4;
class SkinnedMesh : public Mesh {
public:
	SkinnedMesh(const aiMesh* mesh, const aiMaterial* material, const aiNode* root, const aiAnimation* animation);
	virtual ~SkinnedMesh() = default;

	void setTicksPerSecond(float tps);
	void applyBoneTransform(float time_s);
	const glm::mat4& getBoneTransform(size_t index) const;
	const size_t getNumBones() const;

private:
	struct Bone {
		std::string name;
		glm::mat4 offset;
		glm::mat4 globalTransform;
	};
	template <typename T>
	struct Key {
		float time;
		T value;

		Key() : value(glm::uninitialize) {}
		Key(float time_, T value_) : time(time_), value(value_) {}
	};
	struct Node {
		std::shared_ptr<Bone> bone;
		glm::mat4 globalTransform;
		std::vector<Key<glm::vec3>> positionKeys, scaleKeys;
		std::vector<Key<glm::quat>> rotationKeys;
		std::vector<std::shared_ptr<Node>> children;
	};
	std::vector<std::shared_ptr<Bone>> bones_;
	std::unordered_map<std::string, std::shared_ptr<Bone>> nameToBone_;

	std::shared_ptr<Node> rootNode_;
	std::unordered_map<std::string, std::shared_ptr<Node>> nameToNode_;

	float ticksPerSecond_;
	const float duration_;

	GLuint boneIDBuffer_, weightBuffer_;
	std::unique_ptr<unsigned int[][NUM_BONES_PER_VERTEX]> boneIDs_;
	std::unique_ptr<float[][NUM_BONES_PER_VERTEX]> weights_;

	void uploadImpl() override;

	std::shared_ptr<Node> constructNodeTree(const aiNode* aNode, const aiAnimation* animation);
	void processNodeTree(float time, std::shared_ptr<Node> node, const glm::mat4& parentTranform);
	glm::vec3 calcInterpolatedPosition(float time, std::shared_ptr<Node> node) const;
	glm::vec3 calcInterpolatedScale(float time, std::shared_ptr<Node> node) const;
	glm::quat calcInterpolatedRotation(float time, std::shared_ptr<Node> node) const;
};

}
