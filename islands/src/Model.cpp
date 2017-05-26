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

const BoundingBox& Model::getBoundingBox() const {
	return boundingBox_;
}

void Model::loadImpl() {
	static Assimp::Importer importer;
	importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_CAMERAS | aiComponent_LIGHTS);
	const auto scene = importer.ReadFile(filename_,
		aiProcess_GenNormals | aiProcess_ImproveCacheLocality | aiProcess_JoinIdenticalVertices |
		aiProcess_LimitBoneWeights | aiProcess_OptimizeMeshes | aiProcess_RemoveComponent |
		aiProcess_Triangulate);
	if (!scene) {
		SLOG << "Assimp: " << importer.GetErrorString() << std::endl;
		throw;
	}

	assert(scene->HasMaterials());
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
			boundingBox_.expand(mesh->vertices_[iVert]);
		}
	}
	importer.FreeScene();
}

ModelDrawer::ModelDrawer(std::shared_ptr<Model> model) :
	Resource(NameGenerator::generate("ModelDrawer")),
	model_(model),
	visible_(true),
	castShadow_(true),
	receiveShadow_(true),
	animPlaying_(false),
	animStartTime_(0.f) {}

void ModelDrawer::draw() {
	if (animPlaying_) {
		if (!animLoop_ && (glfwGetTime() - animStartTime_) > animDuration_) {
			animPlaying_ = false;
		}
	}

	if (visible_) {
		std::stringstream ss;
		for (const auto mesh : model_->getMeshes()) {
			if (lightmap_) {
				mesh->getMaterial()->setLightmapTexture(lightmap_);
			}

			const auto program = mesh->getMaterial()->getProgram();
			program->use();
			program->setUniform("MVP",
				Camera::getInstance().getProjectionViewMatrix() * getEntity().getModelMatrix());

			if (const auto skinned = std::dynamic_pointer_cast<SkinnedMesh>(mesh)) {
				if (animPlaying_) {
					skinned->updateBoneTransform(static_cast<float>(glfwGetTime()) - animStartTime_);
				}

				for (size_t i = 0; i < skinned->getNumBones(); ++i) {
					ss.str("");
					ss << "bones[" << i << "]";
					program->setUniform(ss.str().c_str(), skinned->getBoneTransform(i));
				}
			}
			mesh->draw();
		}
	}
}

bool ModelDrawer::isLoaded() const {
	return Resource::isLoaded() && model_->isLoaded() && (!lightmap_ || lightmap_->isLoaded());
}

void ModelDrawer::setVisible(bool visible) {
	visible_ = visible;
}

void ModelDrawer::setCastShadow(bool castShadow) {
	castShadow_ = castShadow;
}

void ModelDrawer::setReceiveShadow(bool receiveShadow) {
	receiveShadow_ = receiveShadow;
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

void ModelDrawer::loadImpl() {
	model_->loadAsync();
	if (lightmap_) {
		lightmap_->loadAsync();
	}
}

};
