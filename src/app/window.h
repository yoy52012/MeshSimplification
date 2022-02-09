#pragma once

#include <functional>
#include <string_view>
#include <utility>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

namespace app {

	/** \brief An abstraction for a GLFW window. */
	class Window {

	public:
		/**
		 * \brief Initializes a window.
		 * \param title The window title.
		 * \param window_dimensions The window width and height.
		 * \param opengl_version The OpenGL major and minor version.
		 */
		Window(
			std::string_view title,
			const std::pair<const int, const int>& window_dimensions,
			const std::pair<const int, const int>& opengl_version);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;

		Window(Window&&) noexcept = delete;
		Window& operator=(Window&&) noexcept = delete;

		[[nodiscard]] const std::function<void(int)>& on_key_press() const noexcept { return on_key_press_; }
		void set_on_key_press(const std::function<void(int)>& on_key_press) { on_key_press_ = on_key_press; }

		/**
		 * \brief Gets the window size.
		 * \return A pair representing the window width and height.
		 */
		[[nodiscard]] std::pair<int, int> GetSize() const noexcept;

		/**
		 * \brief Gets the cursor position.
		 * \return The (x,y) coordinates of the cursor in the window.
		 */
		[[nodiscard]] glm::dvec2 GetCursorPosition() const noexcept;

		/**
		 * \brief Determines if the window is closed.
		 * \return \c true if the window is closed, otherwise \c false.
		 */
		[[nodiscard]] bool IsClosed() const noexcept;

		/**
		 * \brief Determines if a key is pressed.
		 * \param key_code The key code to evaluate (e.g., GLFW_KEY_W).
		 * \return \c true if \p key is pressed, otherwise \c false.
		 */
		[[nodiscard]] bool IsKeyPressed(const int key_code) const noexcept;

		/**
		 * \brief Determines if a mouse button is pressed.
		 * \param button_code The mouse button code (e.g., GLFW_MOUSE_BUTTON_LEFT).
		 * \return \c true if \p button is pressed, otherwise \c false.
		 */
		[[nodiscard]] bool IsMouseButtonPressed(const int button_code) const noexcept;

		void SetWindowSizeCallback(const std::function<void(int, int)>& resize_callback) noexcept
		{
			on_resize_callback_ = resize_callback;
		}

		void SetKeyCallback(const std::function<void(int, int, int, int)>& key_callback) noexcept
		{
			on_key_callback_ = key_callback;
		}

		void SetMouseButtonCallback(const std::function<void(int, int, int)>& mouse_button_callback) noexcept
		{
			on_mouse_button_callback_ = mouse_button_callback;
		}

		void SetCursorPosCallback(const std::function<void(double, double)>& cursor_callback) noexcept
		{
			on_cursor_callback_ = cursor_callback;
		}

		[[nodiscard]] const std::function<void(int, int)>& on_resize_callback() const noexcept 
		{ 
			return on_resize_callback_;
		}

        [[nodiscard]] const std::function<void(int, int, int, int)>& on_key_callback() const noexcept
        {
            return on_key_callback_;
        }

        [[nodiscard]] const std::function<void(int, int, int)>& on_mouse_button_callback() const noexcept
        {
            return on_mouse_button_callback_;
        }

        [[nodiscard]] const std::function<void(double, double)>& on_cursor_callback() const noexcept
        {
            return on_cursor_callback_;
        }

		/** \brief Updates the window for the next iteration of main render loop. */
		void Update() const noexcept;

		GLFWwindow* getGLFWWindow() const noexcept 
		{
			return window_;
		}

	private:
		GLFWwindow* window_ = nullptr;
	
		std::function<void(int)> on_key_press_;

		std::function<void(int, int)> on_resize_callback_;
		std::function<void(int, int, int, int)> on_key_callback_;
		std::function<void(int, int, int)> on_mouse_button_callback_;
		std::function<void(double, double)> on_cursor_callback_;

	};
}
