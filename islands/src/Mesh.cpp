#pragma once

#include "Mesh.h"
#include "ResourceSystem.h"

namespace islands {

Mesh::Mesh(const aiMesh* mesh, const aiMaterial* material) :
	Resource(mesh->mName.C_Str()),
	numVertices_(mesh->mNumVertices),
	numIndices_(3 * mesh->mNumFaces),
	vertices_(std::make_unique<glm::vec3[]>(mesh->mNumVertices)),
	normals_(std::make_unique<glm::vec3[]>(mesh->mNumVertices)),
	indices_(std::make_unique<unsigned int[]>(3 * mesh->mNumFaces)),
	hasUV_(mesh->HasTextureCoords(0)) {

	assert(mesh->HasNormals());

	if (hasUV_) {
		uvs_ = std::make_unique<glm::vec2[]>(numVertices_);
	}

	for (unsigned int i = 0; i < numVertices_; ++i) {
		const auto pos = mesh->mVertices[i];
		vertices_[i] = {pos.x, pos.y, pos.z};

		const auto normal = mesh->mNormals[i];
		normals_[i] = {normal.x, normal.y, normal.z};

		if (hasUV_) {
			const auto uvw = mesh->mTextureCoords[0][i];
			uvs_[i] = {uvw.x, uvw.y};
		}
	}

	for (unsigned int iFace = 0; iFace < mesh->mNumFaces; ++iFace) {
		const auto face = mesh->mFaces[iFace];
		assert(face.mNumIndices == 3);
		for (unsigned int iIndex = 0; iIndex < face.mNumIndices; ++iIndex) {
			indices_[3 * iFace + iIndex] = face.mIndices[iIndex];
		}
	}

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
		glDeleteVertexArrays(1, &vertexArray_);
	}
}

bool Mesh::isLoaded() const {
	return Resource::isLoaded() && material_->isLoaded();
}

void Mesh::draw() {
	upload();

	glBindVertexArray(vertexArray_);
	material_->use();
	glDrawElements(GL_TRIANGLES, numIndices_, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

bool Mesh::hasUV() const {
	return hasUV_;
}

std::shared_ptr<Material> Mesh::getMaterial() const {
	return material_;
}

void Mesh::loadImpl() {
	material_->loadAsync();
}

void Mesh::uploadImpl() {
	glGenVertexArrays(1, &vertexArray_);
	glBindVertexArray(vertexArray_);

	glGenBuffers(1, &vertexBuffer_);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
	glBufferData(GL_ARRAY_BUFFER, numVertices_ * sizeof(glm::vec3),
		vertices_.get(), GL_STATIC_DRAW);

	glGenBuffers(1, &normalBuffer_);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer_);
	glBufferData(GL_ARRAY_BUFFER, numVertices_ * sizeof(glm::vec3),
		normals_.get(), GL_STATIC_DRAW);
	normals_.reset();

	if (hasUV_) {
		glGenBuffers(1, &uvBuffer_);
		glBindBuffer(GL_ARRAY_BUFFER, uvBuffer_);
		glBufferData(GL_ARRAY_BUFFER, numVertices_ * sizeof(glm::vec2),
			uvs_.get(), GL_STATIC_DRAW);
		uvs_.reset();
	}

	glGenBuffers(1, &indexBuffer_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices_ * sizeof(unsigned int),
		indices_.get(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer_);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer_);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	if (hasUV_) {
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, uvBuffer_);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	}

	glBindVertexArray(0);
}

glm::mat4 aiMatrix4ToGlmMat4(const aiMatrix4x4& mat) {
	glm::mat4 ret;
	for (size_t i = 0; i < 4; ++i) {
		for (size_t j = 0; j < 4; ++j) {
			ret[i][j] = mat[i][j];
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

void p(const aiNode* n) {
	std::cout << n->mName.C_Str() << std::endl;
	for (size_t i = 0; i < n->mNumChildren; ++i) {
		p(n->mChildren[i]);
	}
}

SkinnedMesh::SkinnedMesh(const aiMesh* mesh, const aiMaterial* material, const aiNode* root, const aiAnimation* animation) :
	Mesh(mesh, material),
	ticksPerSecond_(float(animation->mTicksPerSecond == 0 ? 24.f : animation->mTicksPerSecond)),
	duration_(float(animation->mDuration)) {

	assert(mesh->HasBones());

	assert(ResourceSystem::getInstance().getDefaultSkinningProgram());
	getMaterial()->setProgram(ResourceSystem::getInstance().getDefaultSkinningProgram());

	boneIDs_ = std::make_unique<unsigned int[][NUM_BONES_PER_VERTEX]>(numVertices_);
	weights_ = std::make_unique<float[][NUM_BONES_PER_VERTEX]>(numVertices_);

	std::cout << "bones:" << std::endl;
	for (size_t iBone = 0; iBone < mesh->mNumBones; ++iBone) {
		std::cout << mesh->mBones[iBone]->mName.C_Str() << std::endl;
	}
	std::cout << "channels: " << std::endl;
	for (size_t i = 0; i < animation->mNumChannels; ++i) {
		std::cout << animation->mChannels[i]->mNodeName.C_Str() << std::endl;
	}
	std::cout << "nodes:" << std::endl;
	p(root);

	const auto numAddedBones = std::make_unique<size_t[]>(numVertices_);
	for (size_t iBone = 0; iBone < mesh->mNumBones; ++iBone) {
		const auto bone = mesh->mBones[iBone];

		const auto b = std::make_shared<Bone>();
		b->offset = aiMatrix4ToGlmMat4(bone->mOffsetMatrix);
		bones_.emplace_back(b);
		nameToBone_.emplace(bone->mName.C_Str(), b);

		for (size_t iWeight = 0; iWeight < bone->mNumWeights; ++iWeight) {
			const auto& weight = bone->mWeights[iWeight];
			auto& num = numAddedBones[weight.mVertexId];

			boneIDs_[weight.mVertexId][num] = iBone;
			weights_[weight.mVertexId][num] = weight.mWeight;
			++num;
		}
	}
	rootNode_ = constructNodeTree(root, animation);
}

void SkinnedMesh::setTicksPerSecond(float tps) {
	ticksPerSecond_ = tps;
}

void SkinnedMesh::applyBoneTransform(float time_s) {
	const auto time = std::fmod(time_s * ticksPerSecond_, duration_);
	processNodeTree(time, rootNode_, glm::mat4(1.f));
}

const glm::mat4& SkinnedMesh::getBoneTransform(size_t index) const {
	return bones_.at(index)->globalTransform;
}

const size_t SkinnedMesh::getNumBones() const {
	return bones_.size();
}

void SkinnedMesh::uploadImpl() {
	Mesh::uploadImpl();

	glBindVertexArray(vertexArray_);

	glGenBuffers(1, &boneIDBuffer_);
	glBindBuffer(GL_ARRAY_BUFFER, boneIDBuffer_);
	glBufferData(GL_ARRAY_BUFFER, numVertices_ * sizeof(unsigned int[NUM_BONES_PER_VERTEX]),
		boneIDs_.get(), GL_STATIC_DRAW);
	boneIDs_.reset();

	glGenBuffers(1, &weightBuffer_);
	glBindBuffer(GL_ARRAY_BUFFER, weightBuffer_);
	glBufferData(GL_ARRAY_BUFFER, numVertices_ * sizeof(float[NUM_BONES_PER_VERTEX]),
		weights_.get(), GL_STATIC_DRAW);
	weights_.reset();

	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, boneIDBuffer_);
	glVertexAttribPointer(3, 4, GL_UNSIGNED_INT, GL_FALSE, 0, nullptr);

	glEnableVertexAttribArray(4);
	glBindBuffer(GL_ARRAY_BUFFER, weightBuffer_);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, nullptr);

	glBindVertexArray(0);
}

std::shared_ptr<SkinnedMesh::Node> SkinnedMesh::constructNodeTree(const aiNode* aNode, const aiAnimation* animation) {
	const auto node = std::make_shared<Node>();
	nameToNode_.emplace(aNode->mName.C_Str(), node.get());

	const auto name = aNode->mName.C_Str();
	if (nameToBone_.find(name) != nameToBone_.end()) {
		node->bone = nameToBone_.at(name);
	}
	node->globalTransform = aiMatrix4ToGlmMat4(aNode->mTransformation);

	for (size_t iChannel = 0; iChannel < animation->mNumChannels; ++iChannel) {
		const auto channel = animation->mChannels[iChannel];
		if (channel->mNodeName == aNode->mName) {
			for (size_t iPos = 0; iPos < channel->mNumPositionKeys; ++iPos) {
				const auto posKey = channel->mPositionKeys[iPos];
				node->positionKeys.emplace_back(
					float(posKey.mTime), aiVector3DToGlmVec3(posKey.mValue));
			}
			for (size_t iScale = 0; iScale < channel->mNumScalingKeys; ++iScale) {
				const auto scaleKey = channel->mScalingKeys[iScale];
				node->scaleKeys.emplace_back(
					float(scaleKey.mTime), aiVector3DToGlmVec3(scaleKey.mValue));
			}
			for (size_t iRot = 0; iRot < channel->mNumRotationKeys; ++iRot) {
				const auto rotKey = channel->mRotationKeys[iRot];
				node->rotationKeys.emplace_back(
					float(rotKey.mTime), aiQuaternionToGlmQuat(rotKey.mValue));
			}
			break;
		}
	}

	for (size_t i = 0; i < aNode->mNumChildren; ++i) {
		node->children.emplace_back(constructNodeTree(aNode->mChildren[i], animation));
	}

	return node;
}

void SkinnedMesh::processNodeTree(float time, std::shared_ptr<Node> node, const glm::mat4& parentTranform) {
	if (node->bone) {
		const auto& pos = calcInterpolatedPosition(time, node);
		const auto& scale = calcInterpolatedScale(time, node);
		const auto& rotation = calcInterpolatedRotation(time, node);
		const auto local = glm::scale(
			glm::translate(glm::mat4(1.f), pos) * glm::mat4_cast(rotation), scale);

		const auto global = parentTranform * local * node->bone->offset;
		node->bone->globalTransform = global;

		for (const auto child : node->children) {
			processNodeTree(time, child, global);
		}
	} else {
		for (const auto child : node->children) {
			processNodeTree(time, child, glm::mat4(1.f));
		}
	}
}

glm::vec3 SkinnedMesh::calcInterpolatedPosition(float time, std::shared_ptr<Node> node) const {
	if (node->positionKeys.size() == 1) {
		return node->positionKeys.front().value;
	}
            
	Key<glm::vec3> startKey, endKey;
	for (size_t i = 0; i < node->positionKeys.size() - 1; ++i) {
		if (time < node->positionKeys.at(i).time) {
			endKey = node->positionKeys.at(i + 1);
			startKey = node->positionKeys.at(i);
		}
	}
	const float delta = endKey.time - startKey.time;
	const float factor = (time - startKey.time / delta);
	return startKey.value + factor * (endKey.value - startKey.value);
}

glm::vec3 SkinnedMesh::calcInterpolatedScale(float time, std::shared_ptr<Node> node) const {
	if (node->scaleKeys.size() == 1) {
		return node->scaleKeys.front().value;
	}

	Key<glm::vec3> startKey, endKey;
	for (size_t i = 0; i < node->scaleKeys.size() - 1; ++i) {
		if (time < node->scaleKeys.at(i).time) {
			endKey = node->scaleKeys.at(i + 1);
			startKey = node->scaleKeys.at(i);
		}
	}
	const float delta = endKey.time - startKey.time;
	const float factor = (time - startKey.time / delta);
	return startKey.value + factor * (endKey.value - startKey.value);
}

glm::quat SkinnedMesh::calcInterpolatedRotation(float time, std::shared_ptr<Node> node) const {
	if (node->rotationKeys.size() == 1) {
		return node->rotationKeys.front().value;
	}
    
	Key<glm::quat> startKey, endKey;
	for (size_t i = 0; i < node->rotationKeys.size() - 1; ++i) {
		if (time < node->rotationKeys.at(i).time) {
			endKey = node->rotationKeys.at(i + 1);
			startKey = node->rotationKeys.at(i);
		}
	}
	const auto delta = endKey.time - startKey.time;
	const auto factor = (time - startKey.time / delta);

	return glm::normalize(glm::slerp(startKey.value, endKey.value, factor));
}

}
