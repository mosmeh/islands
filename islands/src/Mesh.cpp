#pragma once

#include "Mesh.h"

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

}
