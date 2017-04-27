#include "ResourceSystem.h"
#include "SceneManager.h"
#include "Chunk.h"
#include "PhysicalBody.h"
#include "Log.h"

class Profiler {
public:
	Profiler() :
		prevTime_(-INFINITY),
		lastDeltaTime_(NAN) {}
	virtual ~Profiler() = default;

	using Real = float;

	void markFrame() {
		const auto time = Real(glfwGetTime());
		if (prevTime_ >= 0) {
			lastDeltaTime_ = time - prevTime_;
		}
		prevTime_ = time;
	}

	Real getLastFPS() const {
		return 1.f / lastDeltaTime_;
	}

	Real getLastDeltaTime() const {
		return lastDeltaTime_;
	}

	void enterSection(const std::string& name) {
		if (samples_.find(name) == samples_.end()) {
			samples_.emplace(name, Sample{Real(glfwGetTime()), 0});
		} else {
			samples_.at(name).startTime = Real(glfwGetTime());
		}
	}

	void leaveSection(const std::string& name) {
		samples_.at(name).elapsedTime += Real(glfwGetTime()) - samples_.at(name).startTime;
	}

	Real getElapsedTime(const std::string& name) const {
		return samples_.at(name).elapsedTime;
	}

	void clearSamples() {
		samples_.clear();
	}

private:
	struct Sample {
		Real startTime;
		Real elapsedTime;
	};

	Real prevTime_;
	Real lastDeltaTime_;
	std::unordered_map<std::string, Sample> samples_;
};

void printGLInfo() {
#define GL_PRINT_STRING(name) SLOG << #name << ": " << glGetString(name) << std::endl;

	GL_PRINT_STRING(GL_VENDOR);
	GL_PRINT_STRING(GL_RENDERER);
	GL_PRINT_STRING(GL_VERSION);
	GL_PRINT_STRING(GL_SHADING_LANGUAGE_VERSION);

	SLOG << "GL_EXTENSIONS:" << std::endl;
	GLint numExtensions;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	for (GLint i = 0; i < numExtensions; ++i) {
		SLOG << glGetStringi(GL_EXTENSIONS, i) << std::endl;
	}

#define GL_PRINT_INTEGER(name) \
	glGetIntegerv(name, &data); \
	SLOG << #name << ": " << data << std::endl;

	GLint data;
	GL_PRINT_INTEGER(GL_MAX_TEXTURE_SIZE);
	GL_PRINT_INTEGER(GL_MAX_3D_TEXTURE_SIZE);
	GL_PRINT_INTEGER(GL_MAX_COMBINED_UNIFORM_BLOCKS);
	GL_PRINT_INTEGER(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS);
	GL_PRINT_INTEGER(GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS);
	GL_PRINT_INTEGER(GL_MAX_DRAW_BUFFERS);

#undef GL_PRINT_STRING
#undef GL_PRINT_INTEGER
}

int main() {
	using namespace islands;

	SLOG << "GLFW: Initializing" << std::endl;
	if (!glfwInit()) {
		SLOG << "GLFW: Failed to initialize" << std::endl;
		throw;
	}
	std::atexit([] {
		SLOG << "GLFW: Terminating" << std::endl;
		glfwTerminate();
	});

	glfwSetErrorCallback([](int code, const char* msg) {
		SLOG << "GLFW: " << code << " " << msg << std::endl;
		throw;
	});
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	SLOG << "GLFW: Creating window" << std::endl;
	const auto window = glfwCreateWindow(1280, 720, "test", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	SLOG << "glad: Loading" << std::endl;
	if (!gladLoadGLLoader(GLADloadproc(glfwGetProcAddress))) {
		SLOG << "glad: Failed to load" << std::endl;
		throw;
	}

	glad_set_post_callback([](const char* name, void*, int, ...) {
		const GLenum errorCode = glad_glGetError();
		if (errorCode == GL_NO_ERROR) {
			return;
		}

#define GL_STACK_OVERFLOW 0x0503
#define GL_STACK_UNDERFLOW 0x0504
#define LOG_GL_ERROR(code) case code: \
SLOG << "glad(" << name << "): " << #code << std::endl; return;

		switch (errorCode) {
			LOG_GL_ERROR(GL_INVALID_ENUM);
			LOG_GL_ERROR(GL_INVALID_VALUE);
			LOG_GL_ERROR(GL_INVALID_OPERATION);
			LOG_GL_ERROR(GL_STACK_OVERFLOW);
			LOG_GL_ERROR(GL_STACK_UNDERFLOW);
			LOG_GL_ERROR(GL_OUT_OF_MEMORY);
			LOG_GL_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION);
		default:
			SLOG << "glad(" << name << "): unknown error (" << errorCode << ")" << std::endl;
			return;
		}

#undef GL_STACK_OVERFLOW
#undef GL_STACK_UNDERFLOW
#undef LOG_GL_ERROR
	});

	printGLInfo();

	glfwSetWindowAspectRatio(window, 16, 9);
	glfwSwapInterval(1);
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height) {
		glViewport(0, 0, width, height);
	});

	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}
	glClearColor(0.f, 0.f, 0.5f, 1.f);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int, int action, int) {
		static glm::vec3 pos = {-15, -15, 15};
		switch (key) {
		case GLFW_KEY_ESCAPE:
			if (action == GLFW_PRESS) {
				glfwSetWindowShouldClose(window, true);
			}
			break;
		/*case GLFW_KEY_K:
			pos.x -= 1.f;
			break;
		case GLFW_KEY_J:
			pos.x += 1.f;
			break;
		case GLFW_KEY_LEFT:
			pos.y += 1.f;
			break;
		case GLFW_KEY_RIGHT:
			pos.y -= 1.f;
			break;
		case GLFW_KEY_UP:
			pos.z += 1.f;
			break;
		case GLFW_KEY_DOWN:
			pos.z -= 1.f;
			break;*/
		}
		//SceneManager::getInstance().setCameraPosition(pos);
	});

	//SceneManager::getInstance().setCameraPosition(glm::vec3(30.f));
	SceneManager::getInstance().setCameraPosition({-15.f, -15.f, 15});
	SceneManager::getInstance().lookAt({0, 0, 0});

	ResourceSystem::getInstance().create<Program>("DefaultProgram", "default.vert", "default.frag");
	ResourceSystem::getInstance().setDefaultProgram("DefaultProgram");

	const auto chunk = std::make_shared<Chunk>("chunk", "forest1.json");
	const auto body = chunk->getEntity("Player")->getComponent<PhysicalBody>();

	glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

	std::ostringstream ss;
	Profiler profiler;
	while (!glfwWindowShouldClose(window)) {
		profiler.markFrame();

		profiler.enterSection("update");
		const float speed = 0.2f;
		glm::vec3 v = body->getVelocity();
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			v.x = -speed;
		} else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			v.x = speed;
		}
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			v.y = speed;
		} else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			v.y = -speed;
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			v.z = speed;
		}
		body->setVelocity(v);

		chunk->update();
		profiler.leaveSection("update");

		profiler.enterSection("draw");
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		chunk->draw();
		glfwSwapBuffers(window);
		profiler.leaveSection("draw");

		ss.str("");
		ss << "FPS: " << profiler.getLastFPS() << ", delta: " << profiler.getLastDeltaTime()
			<< ", update: " << profiler.getElapsedTime("update") <<
			", draw: " << profiler.getElapsedTime("draw") << std::endl;
		glfwSetWindowTitle(window, ss.str().c_str());
		profiler.clearSamples();

		glfwPollEvents();
	}

    return EXIT_SUCCESS;
}