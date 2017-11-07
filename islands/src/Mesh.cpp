#pragma once

#include "Mesh.h"
#include "ResourceSystem.h"

//#define PRINT_ANIMATIONS
#define FOREACH(var, ary, num) for (auto var = ary; var < ary + num; ++var)

namespace islands {

Mesh::Mesh(const aiMesh* mesh, const aiMaterial* material) :
	Resource(mesh->mName.C_Str()),
	numVertices_(mesh->mNumVertices),
	vertices_(std::make_unique<glm::vec3[]>(mesh->mNumVertices)),
	normals_(std::make_unique<glm::vec3[]>(mesh->mNumVertices)),
	indices_(std::make_unique<GLuint[]>(3 * mesh->mNumFaces)),
	hasUV_(mesh->HasTextureCoords(0)) {

	assert(mesh->HasNormals());

	if (hasUV_) {
		uvs_ = std::make_unique<glm::vec2[]>(numVertices_);
	}

	for (size_t i = 0; i < numVertices_; ++i) {
		const auto pos = mesh->mVertices[i];
		vertices_[i] = {pos.x, pos.y, pos.z};

		const auto normal = mesh->mNormals[i];
		normals_[i] = {normal.x, normal.y, normal.z};

		if (hasUV_) {
			const auto uvw = mesh->mTextureCoords[0][i];
			uvs_[i] = {uvw.x, uvw.y};
		}
	}

	size_t idx = 0;
	FOREACH (face, mesh->mFaces, mesh->mNumFaces) {
		if (face->mNumIndices == 3) {
			FOREACH(index, face->mIndices, face->mNumIndices) {
				indices_[idx++] = *index;
			}
		}
	}
	numIndices_ = idx;

	aiString materialName;
	material->Get(AI_MATKEY_NAME, materialName);
	material_ = std::make_shared<Material>(materialName.C_Str(), material);
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

bool Mesh::isLoaded() const {
	return Resource::isLoaded() && material_->isLoaded();
}

void Mesh::draw() {
	upload();

	vertexArray_.bind();
	glDrawElements(GL_TRIANGLES, numIndices_, GL_UNSIGNED_INT, nullptr);
}

bool Mesh::hasUV() const {
	return hasUV_;
}

std::shared_ptr<Material> Mesh::getMaterial() const {
	return material_;
}

void Mesh::uploadImpl() {
	vertexArray_.bind();

	glGenBuffers(1, &vertexBuffer_);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
	glBufferData(GL_ARRAY_BUFFER, numVertices_ * sizeof(glm::vec3),
		vertices_.get(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(Location::POSITION);
	glVertexAttribPointer(Location::POSITION, glm::vec3::length(), GL_FLOAT, GL_FALSE, 0, nullptr);

	glGenBuffers(1, &normalBuffer_);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer_);
	glBufferData(GL_ARRAY_BUFFER, numVertices_ * sizeof(glm::vec3),
		normals_.get(), GL_STATIC_DRAW);
	normals_.reset();
	glEnableVertexAttribArray(Location::NORMAL);
	glVertexAttribPointer(Location::NORMAL, glm::vec3::length(), GL_FLOAT, GL_FALSE, 0, nullptr);

	if (hasUV_) {
		glGenBuffers(1, &uvBuffer_);
		glBindBuffer(GL_ARRAY_BUFFER, uvBuffer_);
		glBufferData(GL_ARRAY_BUFFER, numVertices_ * sizeof(glm::vec2),
			uvs_.get(), GL_STATIC_DRAW);
		uvs_.reset();
		glEnableVertexAttribArray(Location::UV);
		glVertexAttribPointer(Location::UV, glm::vec2::length(), GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	glGenBuffers(1, &indexBuffer_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices_ * sizeof(GLuint),
		indices_.get(), GL_STATIC_DRAW);
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

	getMaterial()->setProgram(
		ResourceSystem::getInstance().get<Program>("SkinningProgram"));

	assert(numVertices_ > 0);
	boneData_ = std::make_unique<BoneDataPerVertex[]>(numVertices_);

	std::unordered_map<std::string, std::shared_ptr<Bone>> nameToBone;
	const auto numAddedBones = std::make_unique<size_t[]>(numVertices_);
	for (size_t i = 0; i < mesh->mNumBones; ++i) {
		const auto bone = mesh->mBones[i];

		const auto b = std::make_shared<Bone>();
		b->offset = aiMatrix4ToGlmMat4(bone->mOffsetMatrix);
		bones_.emplace_back(b);
		nameToBone.emplace(bone->mName.C_Str(), b);

		FOREACH (weight, bone->mWeights, bone->mNumWeights) {
			auto& num = numAddedBones[weight->mVertexId];
			assert(num < NUM_BONES_PER_VERTEX);

			boneData_[weight->mVertexId].boneIDs[num] = i;
			boneData_[weight->mVertexId].weights[num] = weight->mWeight;
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
	glBufferData(GL_ARRAY_BUFFER, numVertices_ * sizeof(BoneDataPerVertex),
		boneData_.get(), GL_STATIC_DRAW);
	boneData_.reset();

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
