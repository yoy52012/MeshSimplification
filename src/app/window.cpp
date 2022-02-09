#include "window.h"

#include <format>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace app;
using namespace std;

namespace {

void APIENTRY HandleDebugMessageReceived(
	const GLenum source,
	const GLenum type,
	const GLuint id,
	const GLenum severity,
	const GLsizei /*length*/,
	const GLchar* const message,
	const void* /*user_param*/) {

	if (id == 131185) return;

	string message_source;
	switch (source) {
		case GL_DEBUG_SOURCE_API: message_source = "API";
			break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: message_source = "WINDOW SYSTEM";
			break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: message_source = "SHADER COMPILER";
			break;
		case GL_DEBUG_SOURCE_THIRD_PARTY: message_source = "THIRD PARTY";
			break;
		case GL_DEBUG_SOURCE_APPLICATION: message_source = "APPLICATION";
			break;
		default: message_source = "OTHER";
			break;
	}

	string message_type;
	switch (type) {
		case GL_DEBUG_TYPE_ERROR: message_type = "ERROR";
			break;
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: message_type = "DEPRECATED BEHAVIOR";
			break;
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: message_type = "UNDEFINED BEHAVIOR";
			break;
		case GL_DEBUG_TYPE_PORTABILITY: message_type = "PORTABILITY";
			break;
		case GL_DEBUG_TYPE_PERFORMANCE: message_type = "PERFORMANCE";
			break;
		default: message_type = "OTHER";
			break;
	}

	string message_severity;
	switch (severity) {
		case GL_DEBUG_SEVERITY_HIGH: message_severity = "HIGH";
			break;
		case GL_DEBUG_SEVERITY_MEDIUM: message_severity = "MEDIUM";
			break;
		case GL_DEBUG_SEVERITY_LOW: message_severity = "LOW";
			break;
		case GL_DEBUG_SEVERITY_NOTIFICATION: message_severity = "NOTIFICATION";
			break;
		default: message_severity = "OTHER";
			break;
	}

	cout << "OpenGL Debug (" << id << "): "
		<< "Source: " << message_source << ", "
		<< "Type: " << message_type << ", "
		<< "Severity: " << message_severity << endl
		<< message << endl;
}

void InitializeGlfw(const pair<const int, const int>& opengl_version) {

	if (!glfwInit()) 
		throw runtime_error{"GLFW initialization failed"};

	const auto [major_version, minor_version] = opengl_version;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major_version);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor_version);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	glfwSwapInterval(1);

#ifdef _DEBUG
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
		glfwSetErrorCallback([](const int error_code, const char* const description) {
			cerr << format("GLFW Error ({}): {}\n", error_code, description);
		});
#endif
}

void InitializeGlad() {


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw runtime_error{ "OpenGL initialization failed" };

#if _DEBUG
		cout << "OpenGL version: " << glGetString(GL_VERSION) << ", "
			<< "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(HandleDebugMessageReceived, nullptr);
#endif
}
}

Window::Window(
	const string_view title,
	const pair<const int, const int>& window_dimensions,
	const pair<const int, const int>& opengl_version) 
{
	InitializeGlfw(opengl_version);

	const auto [width, height] = window_dimensions;
	window_ = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
	if (!window_) 
		throw runtime_error{"Window creation failed"};

	glfwSetWindowUserPointer(window_, this);
	glfwMakeContextCurrent(window_);
	glfwSwapInterval(1);
	
	glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* window, const int width, const int height) noexcept {
        int fb_w, fb_h;
        // Get actual framebuffer size.
        glfwGetFramebufferSize(window, &fb_w, &fb_h);

        glViewport(0, 0, fb_w, fb_h);

		if (const auto* const self = static_cast<Window*>(glfwGetWindowUserPointer(window))) {
			self->on_resize_callback_(width, height);
		}
	});

	glfwSetKeyCallback(
		window_,
		[](GLFWwindow* window, const int key, const int /*scancode*/, const int action, const int /*modifiers*/) noexcept {
			if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
				glfwSetWindowShouldClose(window, true);
			}
			if (action == GLFW_PRESS) {
				if (const auto* const self = static_cast<Window*>(glfwGetWindowUserPointer(window))) {
					self->on_key_press()(key);
				}
			}
		});

	glfwSetMouseButtonCallback(
		window_,
		[](GLFWwindow* window, int button, int action, int mods) noexcept {
		   	
			if (const auto* const self = static_cast<Window*>(glfwGetWindowUserPointer(window))) {
				self->on_mouse_button_callback_(button, action, mods);
			}
		});

	glfwSetCursorPosCallback(
		window_,
		[](GLFWwindow* window, double mouse_x, double mouse_y) noexcept {
			
			if (const auto* const self = static_cast<Window*>(glfwGetWindowUserPointer(window))) {
				self->on_cursor_callback_(mouse_x, mouse_y);
			}
	    });

    InitializeGlad();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
}

Window::~Window() {
	if (window_) 
		glfwDestroyWindow(window_);
	glfwTerminate();
}

std::pair<int, int> Window::GetSize() const noexcept
{
    int width, height;
    glfwGetWindowSize(window_, &width, &height);
    return { width, height };
}

glm::dvec2 Window::GetCursorPosition() const noexcept
{
    double x, y;
    glfwGetCursorPos(window_, &x, &y);
    return { x, y };
}

bool Window::IsClosed() const noexcept
{
    return glfwWindowShouldClose(window_);
}

bool Window::IsKeyPressed(const int key_code) const noexcept
{
    return glfwGetKey(window_, key_code) == GLFW_PRESS;
}

bool Window::IsMouseButtonPressed(const int button_code) const noexcept
{
    return glfwGetMouseButton(window_, button_code);
}

void Window::Update() const noexcept
{
    glfwSwapBuffers(window_);
    glfwPollEvents();
}
