﻿#include "Log.h"
#include "Profiler.h"
#include "Window.h"
#include "Input.h"
#include "Scene.h"

namespace islands {

void printSystemInformation() {
	SLOG << "OS: " << sys::getVersionString() << std::endl;
	const auto memStat = sys::getPhysicalMemoryStatus();
	SLOG << "Available physical memory: " << memStat.available_bytes << " / "
		<< memStat.total_bytes << "bytes" << std::endl;

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

}

int main() {
	using namespace islands;

	glfwSetErrorCallback([](int code, const char* msg) {
		SLOG << "GLFW: " << code << " " << msg << std::endl;
		std::exit(EXIT_FAILURE);
	});

	SLOG << "GLFW: Initializing" << std::endl;
	if (!glfwInit()) {
		SLOG << "GLFW: Failed to initialize" << std::endl;
		std::exit(EXIT_FAILURE);
	}

	SLOG << "PortAudio: Initializing" << std::endl;
	{
		const auto error = Pa_Initialize();
		if (error != paNoError) {
			SLOG << "PortAudio: " << Pa_GetErrorText(error) << std::endl;
			std::exit(EXIT_FAILURE);
		}
	}

	std::atexit([] {
		SLOG << "GLFW: Terminating" << std::endl;
		glfwTerminate();

		SLOG << "PortAudio: Terminating" << std::endl;
		Pa_Terminate();
	});
	
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_SAMPLES, 4);
#ifdef _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

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

	Window::getInstance().update();
	printSystemInformation();

	glClearColor(0.0f, 0.0f, 0.0f, 1.f);
	glClearDepth(1.0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_MULTISAMPLE);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Input::getInstance().registerKeyboardCallback([](int key, int action) {
		switch (key) {
		case GLFW_KEY_ESCAPE:
			if (action == GLFW_PRESS) {
				glfwSetWindowShouldClose(Window::getInstance().getHandle(), GLFW_TRUE);
			}
			break;
		case GLFW_KEY_PRINT_SCREEN:
			if (action == GLFW_PRESS) {
				const auto time = sys::getTime();
				char buf[32];
				std::strftime(buf, sizeof(buf), "screenshot%Y%m%d%I%M%S.png", &time);

				Window::getInstance().saveScreenShot(buf);
			}
			break;
		}
	});


	SceneManager::getInstance().changeScene<TitleScene>();

#ifdef _DEBUG
	std::ostringstream ss;
#endif
	while (Window::getInstance().update()) {
#ifdef _DEBUG
		const auto beforeTime = glfwGetTime();
		Profiler::getInstance().markFrame();
		Profiler::getInstance().enterSection("update");
#endif
		Input::getInstance().update();
		SceneManager::getInstance().update();
#ifdef _DEBUG
		Profiler::getInstance().leaveSection("update");
		Profiler::getInstance().enterSection("draw");
#endif
		SceneManager::getInstance().draw();
#ifdef _DEBUG
		Profiler::getInstance().leaveSection("draw");
		ss.str("");
		ss << "FPS: " << Profiler::getInstance().getLastFPS() <<
			", delta: " << Profiler::getInstance().getLastDeltaTime() <<
			", update: " << Profiler::getInstance().getElapsedTime("update") <<
			", draw: " << Profiler::getInstance().getElapsedTime("draw");
		glfwSetWindowTitle(Window::getInstance().getHandle(), ss.str().c_str());
		Profiler::getInstance().clearSamples();
#endif
	}

	return EXIT_SUCCESS;
}
