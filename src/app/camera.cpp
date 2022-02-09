#include "camera.h"

#include <GLFW/glfw3.h>
#include <trackball.h>

namespace app
{
    Camera::Camera(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
        :eye_(eye),
        center_(center),
        up_(up)
    {
        trackball(curr_quat_, 0, 0, 0, 0);
        view_transform_ = glm::lookAt(eye_, center_, up_);
    }

    glm::mat4 Camera::GetViewTansform() const noexcept
    {
        return view_transform_;
    }

    void Camera::ProcessMouseButtonClick(int button, int action, int mods) noexcept
    {
        (void)mods;

        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                mouse_left_pressed_ = true;
                trackball(prev_quat_, 0.0, 0.0, 0.0, 0.0);
            }
            else if (action == GLFW_RELEASE) {
                mouse_left_pressed_ = false;
            }
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (action == GLFW_PRESS) {
                mouse_right_pressed_ = true;
            }
            else if (action == GLFW_RELEASE) {
                mouse_right_pressed_ = false;
            }
        }
        if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
            if (action == GLFW_PRESS) {
                mouse_middle_pressed_ = true;
            }
            else if (action == GLFW_RELEASE) {
                mouse_middle_pressed_ = false;
            }
        }
    }

    void Camera::ProcessMouseMove(double mouse_x, double mouse_y, int width, int height) noexcept
    {
        const float rotScale = 1.0f;
        const float transScale = 2.0f;

        if (mouse_left_pressed_) {
            trackball(prev_quat_, rotScale * (2.0f * prev_mouse_x - width) / (float)width,
                rotScale * (height - 2.0f * prev_mouse_y) / (float)height,
                rotScale * (2.0f * mouse_x - width) / (float)width,
                rotScale * (height - 2.0f * mouse_y) / (float)height);

            add_quats(prev_quat_, curr_quat_, curr_quat_);
        }
        else if (mouse_middle_pressed_) {
            eye_.x -= transScale * (mouse_x - prev_mouse_x) / (float)width;
            center_.x -= transScale * (mouse_x - prev_mouse_x) / (float)width;
            eye_.y += transScale * (mouse_y - prev_mouse_y) / (float)height;
            center_.y += transScale * (mouse_y - prev_mouse_y) / (float)height;
        }
        else if (mouse_right_pressed_) {
            eye_.z += transScale * (mouse_y - prev_mouse_y) / (float)height;
            center_.z += transScale * (mouse_y - prev_mouse_y) / (float)height;
        }

        // Update mouse point
        prev_mouse_x = mouse_x;
        prev_mouse_y = mouse_y;

        UpdateViewMatrix();
    }

    void Camera::UpdateViewMatrix() noexcept
    {
        float mat[4][4];
        glm::mat4 view_mat = glm::lookAt(eye_, center_, up_);
        build_rotmatrix(mat, curr_quat_);
        glm::mat4 rotate_mat(
            mat[0][0], mat[0][1], mat[0][2], mat[0][3],
            mat[1][0], mat[1][1], mat[1][2], mat[1][3],
            mat[2][0], mat[2][1], mat[2][2], mat[2][3],
            mat[3][0], mat[3][1], mat[3][2], mat[3][3]
        );
        view_transform_ = view_mat * rotate_mat;
    }
}


