#pragma once

#include <array>
#include <vector>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "window.h"
#include "graphics/material.h"
#include "graphics/mesh.h"
#include "graphics/shader_program.h"

namespace app {

class Scene {

public:
	Scene(Window& window, gfx::ShaderProgram& shader_program);
	void Render(float delta_time);

public:
	struct  ViewFrustum {
		float field_of_view_y;
		float z_near;
		float z_far;
	};

	struct Camera {
		glm::vec3 eye;
		glm::vec3 center;
		glm::vec3 up;
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
	void HandleDiscreteKeyPress(int key_code);
	void HandleContinuousInput(float delta_time);


	Window& window_;
	gfx::ShaderProgram& shader_program_;
	std::vector<SceneObject> scene_objects_;
	int active_scene_object_ = 0;
	glm::mat4 view_transform_;
};
}
