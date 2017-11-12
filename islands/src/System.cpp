#include "System.h"

#ifdef _WIN32
#include <VersionHelpers.h>
#include <mmsystem.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw3native.h>
#endif

namespace islands {
namespace sys {

tm getTime() {
	time_t t;
	std::time(&t);
#ifdef _MSC_VER
	tm time;
	localtime_s(&time, &t);
	return time;
#else
	return *std::localtime(&t);
#endif
}

std::string getVersionString() {
#ifdef _WIN32
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
		str = "Unknown Windows";
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
#else
	return "Unknown System";
#endif
}

void disableIME(GLFWwindow* window) {
#ifdef _WIN32
	ImmAssociateContext(glfwGetWin32Window(window), NULL);
#else
#error not implemented
#endif
}

void sleep(std::chrono::milliseconds duration) {
#ifdef _WIN32
	timeBeginPeriod(1);
	Sleep(static_cast<DWORD>(duration.count()));
	timeEndPeriod(1);
#else
#error not implemented
#endif
}

MemoryStatus getPhysicalMemoryStatus() {
#ifdef _WIN32
	MEMORYSTATUSEX stat;
	ZeroMemory(&stat, sizeof(stat));
	stat.dwLength = sizeof(stat);
	GlobalMemoryStatusEx(&stat);
	return MemoryStatus{stat.ullAvailPhys, stat.ullTotalPhys};
#else
#error not implemented
#endif
}

}
}
