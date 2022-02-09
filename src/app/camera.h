#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace app
{
    class Camera
    {
    public:
        explicit Camera(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f));

        [[nodiscard]] glm::mat4 GetViewTansform() const noexcept;

        void ProcessMouseButtonClick(int button, int action, int mods) noexcept;

        void ProcessMouseMove(double mouse_x, double mouse_y, int width, int height) noexcept;

    private:
        void UpdateViewMatrix() noexcept;

    public:
        glm::vec3 eye_;
        glm::vec3 center_;
        glm::vec3 up_;

    private:
        glm::mat4 view_transform_ = glm::mat4(1.0f);

        double prev_mouse_x = 0.0;
        double prev_mouse_y = 0.0;
        bool mouse_left_pressed_ = false;
        bool mouse_middle_pressed_ = false;
        bool mouse_right_pressed_ = false;
        float curr_quat_[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        float prev_quat_[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    };
}
