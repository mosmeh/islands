#pragma once

#include <cassert>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <string>
#include <sstream>
#include <iterator>
#include <vector>
#include <array>
#include <stack>
#include <queue>
#include <unordered_map>
#include <set>
#include <memory>
#include <functional>
#include <future>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <glad/glad.h>
#include <glfw3.h>

#pragma warning(push)
#pragma warning(disable: 4201)
#define GLM_FORCE_SWIZZLE
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <gtc/quaternion.hpp>
#include <gtx/io.hpp>
#pragma warning(pop)

#pragma warning(push)
#pragma warning(disable: 4819)
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma warning(pop)

#include <picojson/picojson.h>

#include <portaudio/portaudio.h>

#define UNUSED(x) (static_cast<void>(x))