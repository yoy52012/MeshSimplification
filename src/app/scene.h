#pragma once

#include <array>
#include <vector>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "window.h"
#include "camera.h"
#include "graphics/material.h"
#include "graphics/mesh.h"
#include "graphics/shader_program.h"

namespace app {

class Scene {

public:
	Scene(Window& window, Camera& camera, gfx::ShaderProgram& shader_program);
	void LoadObject(const std::string_view filepath) noexcept;
	void Render(float delta_time);

public:
	struct  ViewFrustum {
		float field_of_view_y;
		float z_near;
		float z_far;
	};

	struct SceneObject {
		gfx::Mesh mesh;
		gfx::Material material;
	};

	struct PointLight {
		glm::vec4 position;
		glm::vec3 color;
		glm::vec3 attenuation;
	};

private:
	void UpdateProjectionTransform();
	void HandleKeyPress(int key_code);
	void HandleMouseButtonClick(int button, int action, int mods);
	void HandleMouseMove(double mouse_x, double mouse_y);
	void HandleContinuousInput(float delta_time);


	Window& window_;
	Camera& camera_;

	gfx::ShaderProgram& shader_program_;
	std::vector<SceneObject> scene_objects_;
	int active_scene_object_ = 0;


};
}
