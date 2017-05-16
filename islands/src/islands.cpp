﻿#include "ResourceSystem.h"
#include "Camera.h"
#include "InputSystem.h"
#include "Chunk.h"
#include "PhysicalBody.h"
#include "Log.h"

#ifdef _WIN32
#include <VersionHelpers.h>
#include <mmsystem.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw3native.h>
#endif

namespace islands {

class Profiler {
public:
	Profiler() :
		prevTime_(-INFINITY),
		lastDeltaTime_(NAN) {}
	virtual ~Profiler() = default;

	using Real = float;

	void markFrame() {
		const auto time = static_cast<Real>(glfwGetTime());
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
			samples_.emplace(name, Sample{static_cast<Real>(glfwGetTime()), 0});
		} else {
			samples_.at(name).startTime = static_cast<Real>(glfwGetTime());
		}
	}

	void leaveSection(const std::string& name) {
		samples_.at(name).elapsedTime += static_cast<Real>(glfwGetTime()) - samples_.at(name).startTime;
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

#ifdef _WIN32
std::string getWindowsVersionString() {
	std::string str;

	if (IsWindowsVersionOrGreater(10, 0, 0)) {
		str = "Windows 10";
	} else if (IsWindows8Point1OrGreater()) {
		str = "Windows 8.1";
	} else if (IsWindows8OrGreater()) {
		str = "Windows 8";
	} else if (IsWindows7SP1OrGreater()) {
		str = "Windows 7 SP1";
	} else if (IsWindows7OrGreater()) {
		str = "Windows 7";
	} else if (IsWindowsVistaSP2OrGreater()) {
		str = "Windows Vista SP2";
	} else if (IsWindowsVistaSP1OrGreater()) {
		str = "Windows Vista SP1";
	} else if (IsWindowsXPSP3OrGreater()) {
		str = "Windows XP SP3";
	} else if (IsWindowsXPSP2OrGreater()) {
		str = "Windows XP SP2";
	} else if (IsWindowsXPSP1OrGreater()) {
		str = "Windows XP SP1";
	} else if (IsWindowsXPOrGreater()) {
		str = "Windows XP";
	} else {
		str = "Unknown";
	}

	if (IsWindowsServer()) {
		str += " Server";
	}

	BOOL isWow64Process;
	if (IsWow64Process(GetCurrentProcess(), &isWow64Process)) {
		if (isWow64Process) {
			str += " 64bit";
		} else {
			str += " 32bit";
		}
	}

	return str;
}
#endif

void printSystemInformation() {
#ifdef _WIN32
	SLOG << "OS: " << getWindowsVersionString() << std::endl;

	MEMORYSTATUSEX stat;
	stat.dwLength = sizeof(stat);
	if (GlobalMemoryStatusEx(&stat)) {
		SLOG << "Available physical memory: " << stat.ullAvailPhys << " / "
			<< stat.ullTotalPhys << "bytes" << std::endl;
	}
#endif

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

void saveScreenShot(GLFWwindow* window) {
	const int numComponents = 3;
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	size_t size = numComponents * width * height;

	// TODO: 次を調べる
	// 画面サイズが2のべき乗でないときうまく動かない
	// size だけ確保するとアクセス違反を起こす
	const auto pixels = std::make_unique<char[]>(4 * width * height);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.get());
	for (int y = 0; y < height / 2; ++y) {
		// swap_ranges を使うと unsafe だと怒られる
		for (int x = 0; x < width * numComponents; ++x) {
			std::swap(pixels[x + y * width * numComponents],
				pixels[x + (height - y - 1) * width * numComponents]);
		}
	}

	time_t t;
	std::time(&t);
#ifdef _MSC_VER
	struct tm time;
	localtime_s(&time, &t);
#else
	const auto time = *std::localtime(&t);
#endif

	char buf[32];
	strftime(buf, sizeof(buf), "screenshot%Y%m%d%I%M%S.pbm", &time);

	SLOG << "Screen shot: Saving to " << buf << std::endl;
	std::ofstream ofs(buf, std::ios::binary);
	ofs << "P6" << std::endl
		<< width << " " << height << std::endl
		<< 255 << std::endl;
	ofs.write(pixels.get(), size);
}

}

int main() {
	using namespace islands;

	glfwSetErrorCallback([](int code, const char* msg) {
		SLOG << "GLFW: " << code << " " << msg << std::endl;
		throw;
	});

	SLOG << "GLFW: Initializing" << std::endl;
	if (!glfwInit()) {
		SLOG << "GLFW: Failed to initialize" << std::endl;
		throw;
	}
	std::atexit([] {
		SLOG << "GLFW: Terminating" << std::endl;
		glfwTerminate();
	});
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

	SLOG << "GLFW: Creating window" << std::endl;
	const auto window = glfwCreateWindow(1280, 720, "test", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	SLOG << "glad: Loading" << std::endl;
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
		SLOG << "glad: Failed to load" << std::endl;
		throw;
	}

#ifdef _DEBUG
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
#endif

	printSystemInformation();

#ifdef _WIN32
	ImmAssociateContext(glfwGetWin32Window(window), NULL);
#endif

	glfwSetWindowAspectRatio(window, 16, 9);
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow*, int width, int height) {
		glViewport(0, 0, width, height);
	});
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

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

	InputSystem::getInstance().setWindow(window);
	InputSystem::getInstance().registerKeyboardCallback(
		[](GLFWwindow* window, int key, int, int action, int) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			if (action == GLFW_PRESS) {
				glfwSetWindowShouldClose(window, true);
			}
			break;
		case GLFW_KEY_PRINT_SCREEN:
			if (action == GLFW_PRESS) {
				saveScreenShot(window);
			}
			break;
		}
	});

	ResourceSystem::getInstance().setDefaultProgram(ResourceSystem::ProgramType::Default,
		ResourceSystem::getInstance().createOrGet<Program>("DefaultProgram", "default.vert", "default.frag"));
	ResourceSystem::getInstance().setDefaultProgram(ResourceSystem::ProgramType::Skinning,
		ResourceSystem::getInstance().createOrGet<Program>("DefaultSkinningProgram", "skinning.vert", "default.frag"));
	ResourceSystem::getInstance().setDefaultProgram(ResourceSystem::ProgramType::Lightmap,
	ResourceSystem::getInstance().createOrGet<Program>("LightmapProgram", "default.vert", "lightmap.frag"));

	const auto chunk = std::make_shared<Chunk>("chunk", "forest1.json");

	std::ostringstream ss;
	Profiler profiler;
	while (!glfwWindowShouldClose(window)) {
		const auto beforeTime = glfwGetTime();
		profiler.markFrame();

		profiler.enterSection("update");
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

		const auto sleepDuration = 1000 * (1.0 / 60 - glfwGetTime() + beforeTime);
		if (sleepDuration > 0) {
#ifdef _WIN32
			timeBeginPeriod(1);
			Sleep(static_cast<DWORD>(sleepDuration));
			timeEndPeriod(1);
#else
#error not implemented
#endif
		}
	}

    return EXIT_SUCCESS;
}