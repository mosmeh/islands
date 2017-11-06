#include "Model.h"
#include "Entity.h"
#include "Camera.h"
#include "Log.h"
#include "NameGenerator.h"

namespace islands {

Model::Model(const std::string& name, const std::string& filename) :
	Resource(name),
	filename_(filename) {}

const std::vector<std::shared_ptr<Mesh>>& Model::getMeshes() {
	load();
	return meshes_;
}

const geometry::AABB& Model::getLocalAABB() {
	load();
	return localAABB_;
}

void Model::loadImpl() {
	static const std::string MESH_DIR = "mesh";

	static Assimp::Importer importer;
	importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, SkinnedMesh::NUM_BONES_PER_VERTEX);
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_CAMERAS | aiComponent_LIGHTS);
	const auto scene = importer.ReadFile(MESH_DIR + sys::getFilePathSeperator() + filename_,
		aiProcess_GenNormals | aiProcess_ImproveCacheLocality | aiProcess_JoinIdenticalVertices |
		aiProcess_LimitBoneWeights | aiProcess_OptimizeMeshes | aiProcess_RemoveComponent |
		aiProcess_Triangulate);
	if (!scene) {
		SLOG << "Assimp: " << importer.GetErrorString() << std::endl;
		std::exit(EXIT_FAILURE);
	}

	assert(scene->HasMaterials());
	localAABB_.min = glm::vec3(INFINITY);
	localAABB_.max = glm::vec3(-INFINITY);
	for (unsigned int iMesh = 0; iMesh < scene->mNumMeshes; ++iMesh) {
		const auto aMesh = scene->mMeshes[iMesh];
		const auto material = scene->mMaterials[aMesh->mMaterialIndex];

		std::shared_ptr<Mesh> mesh;
		if (aMesh->HasBones()) {
			assert(scene->HasAnimations());
			mesh = std::make_shared<SkinnedMesh>(
				aMesh, material, scene->mRootNode, scene->mAnimations, scene->mNumAnimations);
		} else {
			mesh = std::make_shared<Mesh>(aMesh, material);
		}
		meshes_.emplace_back(mesh);

		for (size_t iVert = 0; iVert < mesh->numVertices_; ++iVert) {
			const auto& vert = mesh->vertices_[iVert];
			localAABB_.min = glm::min(localAABB_.min, vert);
			localAABB_.max = glm::max(localAABB_.max, vert);
		}
	}
	importer.FreeScene();
}

ModelDrawer::ModelDrawer(std::shared_ptr<Model> model) :
	model_(model),
	visible_(true),
	animPlaying_(false),
	animStartTime_(0.f) {}

void ModelDrawer::update() {
	const auto elapsedTime = static_cast<float>(glfwGetTime() - animStartTime_);
	if (animPlaying_) {
		if (!animLoop_ && elapsedTime > animDuration_) {
			animPlaying_ = false;
		}
	}

	for (const auto mesh : model_->getMeshes()) {
		if (visible_ && lightmap_) {
			mesh->getMaterial()->setLightmapTexture(lightmap_);
		}
		if (animPlaying_) {
			if (const auto skinned = std::dynamic_pointer_cast<SkinnedMesh>(mesh)) {
				skinned->updateBoneTransform(elapsedTime);
			}
		}
	}
}

void ModelDrawer::draw() {
	if (visible_) {
		const auto MVP = getEntity().calculateMVPMatrix();
		std::stringstream ss;
		for (const auto mesh : model_->getMeshes()) {
			const auto material = mesh->getMaterial();
			material->use();
			material->getProgram()->setUniform("MVP", MVP);

			if (const auto skinned = std::dynamic_pointer_cast<SkinnedMesh>(mesh)) {
				skinned->applyBoneTransform(material->getProgram());
			}

			mesh->draw();
		}
	}
}

void ModelDrawer::setVisible(bool visible) {
	visible_ = visible;
}

void ModelDrawer::setLightmapTexture(std::shared_ptr<Texture2D> texture) {
	lightmap_ = texture;
}

void ModelDrawer::enableAnimation(const std::string& name, bool loop, float tps) {
	if (!animPlaying_ || name != animName_) {
		for (const auto mesh : model_->getMeshes()) {
			if (const auto skinned = std::dynamic_pointer_cast<SkinnedMesh>(mesh)) {
				skinned->setPlayingAnimation(name);
				skinned->setPlayingAnimationTicksPerSecond(tps);
				animDuration_ = skinned->getPlayingAnimationDurationInSeconds();
			}
		}
		animPlaying_ = true;
		animName_ = name;
		animLoop_ = loop;
		animStartTime_ = float(glfwGetTime());
	} else if (loop != animLoop_) {
		animLoop_ = loop;
	}
}

void ModelDrawer::stopAnimation() {
	animPlaying_ = false;
}

bool ModelDrawer::isPlayingAnimation() const {
	return animPlaying_;
}

};
