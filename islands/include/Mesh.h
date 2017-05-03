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
	enum Location : GLuint {
		POSITION = 0,
		NORMAL = 1,
		UV = 2
	};

	GLuint vertexBuffer_, indexBuffer_, normalBuffer_, uvBuffer_;
	std::unique_ptr<glm::vec3[]> vertices_, normals_;
	std::unique_ptr<glm::vec2[]> uvs_;
	std::unique_ptr<unsigned int[]> indices_;
	const size_t numIndices_;
	const bool hasUV_;
	std::shared_ptr<Material> material_;

	void loadImpl() override;
};

class SkinnedMesh : public Mesh {
public:
	SkinnedMesh(const aiMesh* mesh, const aiMaterial* material, const aiNode* root, const aiAnimation* animation);
	virtual ~SkinnedMesh();

	void setTicksPerSecond(float tps);
	void applyBoneTransform(float time_s);
	const glm::mat4& getBoneTransform(size_t index) const;
	const size_t getNumBones() const;

private:
	struct Bone {
		glm::mat4 offset;
		glm::mat4 transform;
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
		glm::mat4 transform;
		bool hasKeys = false;
		std::vector<Key<glm::vec3>> positionKeys, scaleKeys;
		std::vector<Key<glm::quat>> rotationKeys;
		std::vector<std::shared_ptr<Node>> children;
	};

	static const size_t NUM_BONES_PER_VERTEX = 4;
	static const size_t NUM_MAX_BONES = 128;

	struct BoneDataPerVertex {
		unsigned int boneIDs[NUM_BONES_PER_VERTEX];
		float weights[NUM_BONES_PER_VERTEX];
	};

	enum SkinningLocation : GLuint {
		BONE = 3,
		WEIGHT = 4
	};

	float ticksPerSecond_;
	const float duration_;

	glm::mat4 globalInverse_;

	std::vector<std::shared_ptr<Bone>> bones_;
	std::shared_ptr<Node> rootNode_;

	GLuint boneBuffer_;
	std::unique_ptr<BoneDataPerVertex[]> boneData_;

	void uploadImpl() override;

	std::shared_ptr<Node> constructNodeTree(const aiNode* aNode, const aiAnimation* animation,
		const std::unordered_map<std::string, std::shared_ptr<Bone>>& nameToBone);
	void processNodeTree(float time, std::shared_ptr<Node> node, const glm::mat4& parentTranform);

	template <typename T>
	T getValueAt(float time, const std::vector<Key<T>>& keys);

	template <typename T>
	T interpolate(const T& x, const T& y, float a);
};

template <typename T>
inline T SkinnedMesh::getValueAt(float time, const std::vector<Key<T>>& keys) {
	assert(keys.size() > 0);

	if (keys.size() == 1) {
		return keys.front().value;
	}

	auto keyIter = keys.end();
	for (auto iter = keys.begin(); iter < keys.end() - 1; ++iter) {
		if (time < (iter + 1)->time) {
			keyIter = iter;
			break;
		}
	}
	assert(keyIter + 1 < keys.end());
	const auto& startKey = *keyIter;
	const auto& endKey = *(keyIter + 1);

	const float a = (time - startKey.time) / (endKey.time - startKey.time);
	assert(0 <= a && a <= 1);

	return interpolate(startKey.value, endKey.value, a);
}

template <>
inline glm::vec3 SkinnedMesh::interpolate(const glm::vec3& x, const glm::vec3& y, float a) {
	return glm::mix(x, y, a);
}

template <>
inline glm::quat SkinnedMesh::interpolate(const glm::quat& x, const glm::quat& y, float a) {
	return glm::normalize(glm::slerp(x, y, a));
}

}
