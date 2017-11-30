#include "Mesh.h"

//#define PRINT_ANIMATIONS
#define FOREACH(var, ary, num) for (auto var = ary; var < ary + num; ++var)

namespace islands {

MeshMaterial::MeshMaterial(const aiMaterial* material) : diffuse_(1.f, 0, 1.f, 1.f) {
	aiString aName;
	material->Get(AI_MATKEY_NAME, aName);
	name_ = aName.C_Str();

	aiColor3D aDiffuse;
	if (material->Get(AI_MATKEY_COLOR_DIFFUSE, aDiffuse) == AI_SUCCESS) {
		diffuse_.rgb = {aDiffuse.r, aDiffuse.g, aDiffuse.b};
	}

	ai_real opacity;
	if (material->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS) {
		diffuse_.a = opacity;
	}
}

const std::string& MeshMaterial::getName() const {
	return name_;
}

const glm::vec4& MeshMaterial::getDiffuse() const {
	return diffuse_;
}

void MeshMaterial::apply(std::shared_ptr<Program> program) const {
	program->use();
	program->setUniform("diffuse", diffuse_);
}

Mesh::Mesh(const aiMesh* mesh, const aiMaterial* material) :
	Resource(mesh->mName.C_Str()),
	vertices_(mesh->mNumVertices),
	normals_(mesh->mNumVertices),
	hasUV_(mesh->HasTextureCoords(0)),
	meshMaterial_(material) {

	assert(mesh->HasNormals());

	if (hasUV_) {
		uvs_ = std::vector<glm::vec2>(mesh->mNumVertices);
	}

	for (size_t i = 0; i < mesh->mNumVertices; ++i) {
		const auto& pos = mesh->mVertices[i];
		vertices_.at(i) = {pos.x, pos.y, pos.z};

		const auto& normal = mesh->mNormals[i];
		normals_.at(i) = {normal.x, normal.y, normal.z};

		if (hasUV_) {
			const auto& uvw = mesh->mTextureCoords[0][i];
			uvs_.at(i) = {uvw.x, uvw.y};
		}
	}

	indices_.reserve(3 * mesh->mNumFaces);
	FOREACH (face, mesh->mFaces, mesh->mNumFaces) {
		if (face->mNumIndices == 3) {
			FOREACH(index, face->mIndices, face->mNumIndices) {
				indices_.emplace_back(*index);
			}
		}
	}
	indices_.shrink_to_fit();
}

Mesh::~Mesh() {
	if (isUploaded()) {
		glDeleteBuffers(1, &vertexBuffer_);
		glDeleteBuffers(1, &normalBuffer_);
		glDeleteBuffers(1, &indexBuffer_);
		if (hasUV_) {
			glDeleteBuffers(1, &uvBuffer_);
		}
	}
}

void Mesh::draw() {
	upload();

	vertexArray_.bind();
	glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, nullptr);
}

bool Mesh::hasUV() const {
	return hasUV_;
}

const MeshMaterial& Mesh::getMeshMaterial() const {
	return meshMaterial_;
}

const std::vector<glm::vec3>& Mesh::getVertices() const {
	return vertices_;
}

const std::vector<GLuint>& Mesh::getIndices() const {
	return indices_;
}

std::vector<geometry::Triangle> Mesh::getTriangles() const {
	std::vector<geometry::Triangle> triangles;
	triangles.reserve(indices_.size());
	for (size_t i = 0; i < indices_.size() / 3; ++i) {
		triangles.emplace_back(geometry::Triangle{
			vertices_.at(indices_.at(3 * i + 0)),
			vertices_.at(indices_.at(3 * i + 1)),
			vertices_.at(indices_.at(3 * i + 2))
		});
	}
	return triangles;
}

void Mesh::uploadImpl() {
	vertexArray_.bind();

	glGenBuffers(1, &vertexBuffer_);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
	glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(glm::vec3),
		vertices_.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(Location::POSITION);
	glVertexAttribPointer(Location::POSITION, glm::vec3::length(), GL_FLOAT, GL_FALSE, 0, nullptr);

	glGenBuffers(1, &normalBuffer_);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer_);
	glBufferData(GL_ARRAY_BUFFER, normals_.size() * sizeof(glm::vec3),
		normals_.data(), GL_STATIC_DRAW);
	normals_.clear();
	glEnableVertexAttribArray(Location::NORMAL);
	glVertexAttribPointer(Location::NORMAL, glm::vec3::length(), GL_FLOAT, GL_FALSE, 0, nullptr);

	if (hasUV_) {
		glGenBuffers(1, &uvBuffer_);
		glBindBuffer(GL_ARRAY_BUFFER, uvBuffer_);
		glBufferData(GL_ARRAY_BUFFER, uvs_.size() * sizeof(glm::vec2),
			uvs_.data(), GL_STATIC_DRAW);
		uvs_.clear();
		glEnableVertexAttribArray(Location::UV);
		glVertexAttribPointer(Location::UV, glm::vec2::length(), GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	glGenBuffers(1, &indexBuffer_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(GLuint),
		indices_.data(), GL_STATIC_DRAW);
}

glm::mat4 aiMatrix4ToGlmMat4(const aiMatrix4x4& mat) {
	glm::mat4 ret;
	for (size_t i = 0; i < 4; ++i) {
		for (size_t j = 0; j < 4; ++j) {
			ret[i][j] = mat[j][i];
		}
	}
	return ret;
}

glm::vec3 aiVector3DToGlmVec3(const aiVector3D& vec) {
	return{vec.x, vec.y, vec.z};
}

glm::quat aiQuaternionToGlmQuat(const aiQuaternion& quat) {
	return{quat.w, quat.x, quat.y, quat.z};
}

SkinnedMesh::SkinnedMesh(const aiMesh* mesh, const aiMaterial* material, const aiNode* root,
	aiAnimation** animations, size_t numAnimations) :
	Mesh(mesh, material),
	playingAnim_(nullptr) {

	assert(mesh->HasBones());

	assert(getVertices().size() > 0);
	boneData_ = std::vector<BoneDataPerVertex>(getVertices().size());

	std::unordered_map<std::string, std::shared_ptr<Bone>> nameToBone;
	std::vector<size_t> numAddedBones(getVertices().size());
	for (size_t i = 0; i < mesh->mNumBones; ++i) {
		const auto bone = mesh->mBones[i];

		const auto b = std::make_shared<Bone>();
		b->offset = aiMatrix4ToGlmMat4(bone->mOffsetMatrix);
		bones_.emplace_back(b);
		nameToBone.emplace(bone->mName.C_Str(), b);

		FOREACH (weight, bone->mWeights, bone->mNumWeights) {
			auto& num = numAddedBones.at(weight->mVertexId);
			assert(num < NUM_BONES_PER_VERTEX);

			boneData_.at(weight->mVertexId).boneIDs[num] = i;
			boneData_.at(weight->mVertexId).weights[num] = weight->mWeight;
			++num;
		}
	}
	assert(bones_.size() <= NUM_MAX_BONES);

	for (size_t i = 0; i < numAnimations; ++i) {
		const auto animation = animations[i];
		const auto anim = std::make_shared <Animation>();
		anim->ticksPerSecond = animation->mTicksPerSecond == 0 ? 24.0 : animation->mTicksPerSecond;
		anim->duration = animation->mDuration;
		anim->rootNode = constructNodeTree(root, animation, nameToBone);
		animations_.emplace(animation->mName.C_Str(), anim);
	}
	globalInverse_ = glm::inverse(animations_.begin()->second->rootNode->transform);

#ifdef PRINT_ANIMATIONS
	std::cout << mesh->mName.C_Str() << std::endl;
	for (const auto& a : animations_) {
		std::cout << "\t" << a.first << std::endl;
	}
#endif
}

SkinnedMesh::~SkinnedMesh() {
	if (isUploaded()) {
		glDeleteBuffers(1, &boneBuffer_);
	}
}

void SkinnedMesh::setPlayingAnimation(const std::string& name) {
	assert(animations_.find(name) != animations_.end());
	playingAnim_ = animations_.at(name);
}

void SkinnedMesh::setPlayingAnimationTicksPerSecond(double tps) {
	assert(playingAnim_);
	playingAnim_->ticksPerSecond = tps;
}

double SkinnedMesh::getPlayingAnimationTicks() const {
	assert(playingAnim_);
	return playingAnim_->duration;
}

void SkinnedMesh::updateBoneTransform(double time_s) {
	assert(playingAnim_);
	const auto time = std::fmod(time_s * playingAnim_->ticksPerSecond, playingAnim_->duration);
	processNodeTree(time, playingAnim_->rootNode, glm::mat4(1.f));
}

void SkinnedMesh::applyBoneTransform(std::shared_ptr<Program> program) const {
	std::stringstream ss;
	program->use();
	for (size_t i = 0; i < bones_.size(); ++i) {
		ss.str("");
		ss << "bones[" << i << "]";
		program->setUniform(ss.str().c_str(), bones_.at(i)->transform);
	}
}

void SkinnedMesh::uploadImpl() {
	Mesh::uploadImpl();

	glGenBuffers(1, &boneBuffer_);
	glBindBuffer(GL_ARRAY_BUFFER, boneBuffer_);
	glBufferData(GL_ARRAY_BUFFER, boneData_.size() * sizeof(BoneDataPerVertex),
		boneData_.data(), GL_STATIC_DRAW);
	boneData_.clear();

	glEnableVertexAttribArray(SkinningLocation::BONE);
	glVertexAttribIPointer(SkinningLocation::BONE, NUM_BONES_PER_VERTEX, GL_UNSIGNED_INT,
		sizeof(BoneDataPerVertex), nullptr);

	glEnableVertexAttribArray(SkinningLocation::WEIGHT);
	glVertexAttribPointer(SkinningLocation::WEIGHT, NUM_BONES_PER_VERTEX, GL_FLOAT, GL_FALSE,
		sizeof(BoneDataPerVertex), reinterpret_cast<GLvoid*>(sizeof(GLfloat) * NUM_BONES_PER_VERTEX));
}

std::shared_ptr<SkinnedMesh::Node> SkinnedMesh::constructNodeTree(const aiNode* aNode,
	const aiAnimation* animation,
	const std::unordered_map<std::string, std::shared_ptr<Bone>>& nameToBone) {

	const auto node = std::make_shared<Node>();

	const auto name = aNode->mName.C_Str();
	if (nameToBone.find(name) != nameToBone.end()) {
		node->bone = nameToBone.at(name);
	}
	node->transform = aiMatrix4ToGlmMat4(aNode->mTransformation);

	FOREACH (pChannel, animation->mChannels, animation->mNumChannels) {
		const auto& channel = **pChannel;
		if (channel.mNodeName == aNode->mName) {
			node->hasKeys = true;
			FOREACH (key, channel.mPositionKeys, channel.mNumPositionKeys) {
				node->positionKeys.emplace_back(
					static_cast<float>(key->mTime), aiVector3DToGlmVec3(key->mValue));
			}
			FOREACH (key, channel.mScalingKeys, channel.mNumScalingKeys) {
				node->scaleKeys.emplace_back(
					static_cast<float>(key->mTime), aiVector3DToGlmVec3(key->mValue));
			}
			FOREACH (key, channel.mRotationKeys, channel.mNumRotationKeys) {
				node->rotationKeys.emplace_back(
					static_cast<float>(key->mTime), aiQuaternionToGlmQuat(key->mValue));
			}
			break;
		}
	}

	FOREACH (child, aNode->mChildren, aNode->mNumChildren) {
		node->children.emplace_back(constructNodeTree(*child, animation, nameToBone));
	}

	return node;
}

void SkinnedMesh::processNodeTree(double time, std::shared_ptr<Node> node, const glm::mat4& parentTranform) {
	auto local = node->transform;
	if (node->hasKeys) {
		const auto& pos = getValueAt(time, node->positionKeys);
		const auto& scale = getValueAt(time, node->scaleKeys);
		const auto& rotation = getValueAt(time, node->rotationKeys);
		local = glm::scale(
			glm::translate(glm::mat4(1.f), pos) * glm::mat4_cast(rotation), scale);
	}
	const auto global = parentTranform * local;
	if (node->bone) {
		node->bone->transform = globalInverse_ * global * node->bone->offset;
	}

	for (const auto child : node->children) {
		processNodeTree(time, child, global);
	}
}

}
