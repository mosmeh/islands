#include "Model.h"
#include "Entity.h"
#include "Camera.h"
#include "AssetArchive.h"
#include "Log.h"

namespace islands {

Model::Model(const std::string& name, const std::string& filename) :
	SharedResource(name),
	filename_(filename),
	opaque_(true),
	hasSkinned_(false) {}

const std::vector<std::shared_ptr<Mesh>>& Model::getMeshes() {
	load();
	return meshes_;
}

bool Model::isOpaque() {
	load();
	return opaque_;
}

bool Model::hasSkinnedMesh() {
	load();
	return hasSkinned_;
}

const geometry::AABB& Model::getLocalAABB() {
	load();
	return localAABB_;
}

void Model::loadImpl() {
	static const std::string MESH_DIR = "mesh";
	static const auto FLAGS = aiProcess_GenNormals | aiProcess_ImproveCacheLocality |
		aiProcess_JoinIdenticalVertices | aiProcess_LimitBoneWeights | aiProcess_OptimizeMeshes |
		aiProcess_RemoveComponent |	aiProcess_Triangulate;
	static Assimp::Importer importer;
	importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, SkinnedMesh::NUM_BONES_PER_VERTEX);
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_CAMERAS | aiComponent_LIGHTS);

#ifdef ENABLE_ASSET_ARCHIVE
	const auto filePath = MESH_DIR + '/' + filename_;
	auto rawData = AssetArchive::getInstance().readFile(filePath);
	const auto scene = importer.ReadFileFromMemory(rawData.data(), rawData.size(), FLAGS);
#else
	const auto filePath = MESH_DIR + sys::getFilePathSeparator() + filename_;
	const auto scene = importer.ReadFile(filePath, FLAGS);
#endif
	if (!scene) {
		SLOG << "Assimp: " << importer.GetErrorString() << std::endl;
		std::exit(EXIT_FAILURE);
	}

	assert(scene->HasMaterials());
	localAABB_.min = glm::vec3(INFINITY);
	localAABB_.max = glm::vec3(-INFINITY);
	for (size_t i = 0; i < scene->mNumMeshes; ++i) {
		const auto aMesh = scene->mMeshes[i];
		const auto material = scene->mMaterials[aMesh->mMaterialIndex];

		std::shared_ptr<Mesh> mesh;
		if (aMesh->HasBones()) {
			assert(scene->HasAnimations());
			mesh = std::make_shared<SkinnedMesh>(
				aMesh, material, scene->mRootNode, scene->mAnimations, scene->mNumAnimations);
			hasSkinned_ = true;
		} else {
			mesh = std::make_shared<Mesh>(aMesh, material);
		}
		meshes_.emplace_back(mesh);

		for (const auto& vert : mesh->getVertices()) {
			localAABB_.min = glm::min(localAABB_.min, vert);
			localAABB_.max = glm::max(localAABB_.max, vert);
		}
		if (mesh->getMaterial().getDiffuse().a < 1.f) {
			opaque_ = false;
		}
	}
	importer.FreeScene();
}

ModelDrawer::ModelDrawer(std::shared_ptr<Model> model) :
	model_(model),
	visible_(true),
	cullFaceEnabled_(true) {

	updateProgram();
}

void ModelDrawer::update() {
	const auto elapsedTime = static_cast<float>(glfwGetTime() - anim_.startTime)
		+ anim_.startFrame / (24.0 * anim_.tps);

	if (anim_.playing) {
		if (!anim_.loop && elapsedTime > anim_.duration) {
			anim_.playing = false;
		} else {
			for (const auto mesh : model_->getMeshes()) {
				if (const auto skinned = std::dynamic_pointer_cast<SkinnedMesh>(mesh)) {
					skinned->updateBoneTransform(elapsedTime);
				}
			}
		}
	}
}

void ModelDrawer::draw() {
	if (visible_) {
		if (!cullFaceEnabled_) {
			glDisable(GL_CULL_FACE);
		}
		if (!isOpaque()) {
			glEnable(GL_BLEND);
		}

		const auto& MVP = getEntity().calculateMVPMatrix();
		if (texture_) {
			texture_->bind(0);
		}

		const auto prepare = [&](std::shared_ptr<Program> program) {
			program->use();
			program->setUniform("MVP", MVP);
			if (texture_) {
				program->setUniform("tex", static_cast<GLuint>(0));
			}
		};

		prepare(program_);
		if (model_->hasSkinnedMesh()) {
			prepare(skinningProgram_);
		};

		for (const auto mesh : model_->getMeshes()) {
			if (const auto skinned = std::dynamic_pointer_cast<SkinnedMesh>(mesh)) {
				mesh->getMaterial().apply(skinningProgram_);
				skinned->applyBoneTransform(skinningProgram_);
			} else {
				mesh->getMaterial().apply(program_);
			}
			mesh->draw();
		}

		if (!cullFaceEnabled_) {
			glEnable(GL_CULL_FACE);
		}
		if (!isOpaque()) {
			glDisable(GL_BLEND);
		}
	}
}

bool ModelDrawer::isOpaque() const {
	return model_->isOpaque();
}

std::shared_ptr<Model> ModelDrawer::getModel() const {
	return model_;
}

void ModelDrawer::setVisible(bool visible) {
	visible_ = visible;
}

void ModelDrawer::setTexture(std::shared_ptr<Texture2D> texture) {
	texture_ = texture;
	updateProgram();
}

void ModelDrawer::setVertexShader(std::shared_ptr<Shader> shader) {
	vertex_ = shader;
	updateProgram();
}

void ModelDrawer::setGeometryShader(std::shared_ptr<Shader> shader) {
	geometry_ = shader;
	updateProgram();
}

void ModelDrawer::setFragmentShader(std::shared_ptr<Shader> shader) {
	fragment_ = shader;
	updateProgram();
}

void ModelDrawer::updateProgram() {
	std::shared_ptr<Shader> fragment = fragment_;
	if (!fragment) {
		std::string name;
		if (texture_) {
			name = "texture.frag";
		} else {
			name = "default.frag";
		}
		fragment = Shader::createOrGet(name, name, GL_FRAGMENT_SHADER);
	}

	const auto getProgram = [&](bool skinning) {
		std::shared_ptr<Shader> vertex = vertex_;
		if (!vertex) {
			std::string name;
			if (skinning) {
				name = "skinning.vert";
			} else {
				name = "default.vert";
			}
			vertex = Shader::createOrGet(name, name, GL_VERTEX_SHADER);
		}
		if (geometry_) {
			return Program::createOrGet(
				vertex->getName() + "//" + geometry_->getName() + "//" + fragment->getName(),
				vertex, geometry_, fragment);
		} else {
			return Program::createOrGet(
				vertex->getName() + "//" + fragment->getName(),
				vertex, fragment);
		}
	};

	program_ = getProgram(false);
	if (model_->hasSkinnedMesh()) {
		skinningProgram_ = getProgram(true);
	}
}

void ModelDrawer::setCullFaceEnabled(bool enabled) {
	cullFaceEnabled_ = enabled;
}

void ModelDrawer::enableAnimation(const std::string& name, bool loop, double tps, size_t startFrame) {
	if (!anim_.playing || name != anim_.name) {
		//anim_ = Animation{name, true, loop, 0.0, tps, glfwGetTime(), startFrame};
		anim_.name = name;
		anim_.playing = true;
		anim_.loop = loop;
		anim_.tps = tps;
		anim_.startTime = glfwGetTime();
		anim_.startFrame = startFrame;
		for (const auto mesh : model_->getMeshes()) {
			if (const auto skinned = std::dynamic_pointer_cast<SkinnedMesh>(mesh)) {
				skinned->setPlayingAnimation(name);
				skinned->setPlayingAnimationTicksPerSecond(tps);
				anim_.duration = skinned->getPlayingAnimationTicks() / tps;
			}
		}
	} else if (loop != anim_.loop) {
		anim_.loop = loop;
	}
}

void ModelDrawer::stopAnimation() {
	anim_.playing = false;
}

bool ModelDrawer::isPlayingAnimation() const {
	return anim_.playing;
}

size_t ModelDrawer::getCurrentAnimationFrame() const {
	return anim_.startFrame
		+ static_cast<size_t>(24.0 * anim_.tps * (glfwGetTime() - anim_.startTime));
}

};
