#include "Model.h"
#include "Entity.h"
#include "SceneManager.h"
#include "Log.h"

namespace islands {

Model::Model(const std::string& name, const std::string& filename) :
	Resource(name),
	filename_(filename) {}

const std::vector<std::shared_ptr<Mesh>>& Model::getMeshes() const {
	return meshes_;
}

void Model::loadImpl() {
	static Assimp::Importer importer;
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_CAMERAS | aiComponent_LIGHTS);
	const auto scene = importer.ReadFile(filename_,
		aiProcess_Triangulate |	aiProcess_GenNormals | aiProcess_ImproveCacheLocality |
		aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes | aiProcess_RemoveComponent);
	if (!scene) {
		SLOG << "Assimp: " << importer.GetErrorString() << std::endl;
		throw;
	}

	assert(scene->HasMaterials());
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
		const auto mesh = scene->mMeshes[i];
		meshes_.emplace_back(std::make_shared<Mesh>(mesh, scene->mMaterials[mesh->mMaterialIndex]));
	}
	importer.FreeScene();
}

ModelDrawer::ModelDrawer(std::shared_ptr<Model> model) :
	model_(model),
	visible_(true),
	castShadow_(true),
	receiveShadow_(true) {}

void ModelDrawer::draw() {
	if (visible_) {
		for (const auto mesh : model_->getMeshes()) {
			mesh->getMaterial()->setLightmapTexture(lightmap_);
			const auto program = mesh->getMaterial()->getProgram();
			program->use();
			program->setUniform("MVP",
				SceneManager::getInstance().getProjectionViewMatrix() *	getEntity().getModelMatrix());
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

void ModelDrawer::loadImpl() {
	model_->loadAsync();
	if (lightmap_) {
		lightmap_->loadAsync();
	}
}

};
