#pragma once

#include "Resource.h"
#include "Material.h"
#include "GLObjects.h"
#include "Geometry.h"

namespace islands {

class Mesh : public Resource {
public:
	Mesh(const aiMesh* mesh, const aiMaterial* material);
	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh(Mesh&& mesh) = default;

	virtual ~Mesh();

	void draw();

	bool hasUV() const;
	const Material& getMaterial() const;

	const std::vector<glm::vec3>& getVertices() const;
	const std::vector<GLuint>& getIndices() const;
	std::vector<geometry::Triangle> getTriangles() const;

protected:
	void uploadImpl() override;

private:
	enum Location : GLuint {
		POSITION = 0,
		NORMAL = 1,
		UV = 2
	};

	VertexArray vertexArray_;
	GLuint vertexBuffer_, indexBuffer_, normalBuffer_, uvBuffer_;
	std::vector<glm::vec3> vertices_, normals_;
	std::vector<glm::vec2> uvs_;
	std::vector<GLuint> indices_;
	const bool hasUV_;
	Material material_;
};

class SkinnedMesh : public Mesh {
public:
	static const size_t NUM_BONES_PER_VERTEX = 4;

	SkinnedMesh(const aiMesh* mesh, const aiMaterial* material, const aiNode* root, aiAnimation** animations, size_t numAnimations);
	virtual ~SkinnedMesh();

	void setPlayingAnimation(const std::string& name);
	void setPlayingAnimationTicksPerSecond(double tps);
	double getPlayingAnimationTicks() const;
	void updateBoneTransform(double time_s);
	void applyBoneTransform(std::shared_ptr<Program> program) const;

private:
	struct Bone {
		glm::mat4 offset;
		glm::mat4 transform;
	};

	template <typename T>
	struct Key {
		float time;
		T value;

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

	static const size_t NUM_MAX_BONES = 128;

	struct BoneDataPerVertex {
		GLuint boneIDs[NUM_BONES_PER_VERTEX] = {};
		GLfloat weights[NUM_BONES_PER_VERTEX] = {};
	};

	enum SkinningLocation : GLuint {
		BONE = 3,
		WEIGHT = 4
	};

	struct Animation {
		double ticksPerSecond;
		double duration;
		std::shared_ptr<Node> rootNode;

		Animation() = default;
	};

	std::unordered_map<std::string, std::shared_ptr<Animation>> animations_;
	std::shared_ptr<Animation> playingAnim_;
	glm::mat4 globalInverse_;
	std::vector<std::shared_ptr<Bone>> bones_;

	GLuint boneBuffer_;
	std::vector<BoneDataPerVertex> boneData_;

	void uploadImpl() override;

	std::shared_ptr<Node> constructNodeTree(const aiNode* aNode, const aiAnimation* animation,
		const std::unordered_map<std::string, std::shared_ptr<Bone>>& nameToBone);
	void processNodeTree(double time, std::shared_ptr<Node> node, const glm::mat4& parentTranform);

	template <typename T>
	T getValueAt(double time, const std::vector<Key<T>>& keys);

	template <typename T>
	T interpolate(const T& x, const T& y, float a);
};

template <typename T>
inline T SkinnedMesh::getValueAt(double time, const std::vector<Key<T>>& keys) {
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

	if (keyIter == keys.end()) {
		return (keys.end() - 1)->value;
	}

	const auto& startKey = *keyIter;
	const auto& endKey = *(keyIter + 1);

	const auto a = (time - startKey.time) / (endKey.time - startKey.time);
	return interpolate(startKey.value, endKey.value, static_cast<float>(a));
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
