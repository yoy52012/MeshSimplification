#include "scene.h"

#include <algorithm>
#include <optional>

#include <glm/gtc/matrix_transform.hpp>

#include "geometry/mesh_simplifier.h"
#include "graphics/arcball.h"
#include "graphics/obj_loader.h"

using namespace app;
using namespace geometry;
using namespace gfx;
using namespace glm;
using namespace std;

static  Scene::ViewFrustum kViewFrustrum{
    .field_of_view_y = glm::radians(45.f),
    .z_near = 0.01f,
    .z_far = 100.0f
};

//static  Scene::Camera kCamera{
//    .eye = glm::vec3{0.0f, 0.0f, 3.0f},
//    .center = glm::vec3{0.0f},
//    .up = glm::vec3{0.0f, 1.0f, 0.0f}
//};

static  std::array<Scene::PointLight, 2> kPointLights{
	Scene::PointLight{
        .position = glm::vec4{1.f, 1.f, 1.f, 1.f},
        .color = glm::vec3{1.f},
        .attenuation = glm::vec3{0.f, 0.f, 1.f}
    },
	Scene::PointLight{
        .position = glm::vec4{-1.f, 1.f, 2.f, 1.f},
        .color = glm::vec3{1.f},
        .attenuation = glm::vec3{0.f, 0.f, 1.f}
    }
};


Scene::Scene(Window& window, Camera& camera, ShaderProgram& shader_program)
	: window_{window}, 
	camera_(camera),
	shader_program_{shader_program}
{
	window.set_on_key_press([this](const auto key_code) { HandleKeyPress(key_code); });
	
    window.SetWindowSizeCallback([this](int width, int height) {HandleWindowResize(width, height); });
	window.SetMouseButtonCallback([this](int button, int action, int mods) { HandleMouseButtonClick(button, action, mods); });
	window.SetCursorPosCallback([this](double mouse_x, double mouse_y) { HandleMouseMove(mouse_x, mouse_y); });

	shader_program_.Enable();
	auto view_transform = camera_.GetViewTansform();

	UpdateProjectionTransform();

	LoadObject(ASSETS_FOLDER"/models/bunny.obj");

	for (size_t i = 0; i < kPointLights.size(); ++i) {
		const auto& [position, color, attenuation] = kPointLights[i];
		shader_program_.SetUniform(format("point_lights[{}].position", i), vec3{ view_transform * position});
		shader_program_.SetUniform(format("point_lights[{}].color", i), color);
		shader_program_.SetUniform(format("point_lights[{}].attenuation", i), attenuation);
	}
}

void Scene::LoadObject(const std::string_view filepath) noexcept
{
    auto mesh = obj_loader::LoadMesh(filepath);

	const auto& bmax = mesh.GetBoxMax();
	const auto& bmin = mesh.GetBoxMin();

    float maxExtent = 0.5f * (bmax[0] - bmin[0]);
    if (maxExtent < 0.5f * (bmax[1] - bmin[1])) {
        maxExtent = 0.5f * (bmax[1] - bmin[1]);
    }
    if (maxExtent < 0.5f * (bmax[2] - bmin[2])) {
        maxExtent = 0.5f * (bmax[2] - bmin[2]);
    }

    mesh.Scale(vec3{ 1.0f / maxExtent });
    mesh.Translate(vec3{ -0.5 * (bmax[0] + bmin[0]), -0.5 * (bmax[1] + bmin[1]), -0.5 * (bmax[2] + bmin[2]) });

    scene_objects_.push_back(SceneObject{
        .mesh = move(mesh),
        .material = Material::FromType(current_mtl_type_)
        });
}

void Scene::SetMaterialType(gfx::MaterialType mtl_type) noexcept
{
    if (current_mtl_type_ != mtl_type) {

        for (auto& scene_object :scene_objects_)
        {
            scene_object.material = Material::FromType(mtl_type);
        }

        current_mtl_type_ = mtl_type;
    }
}

void Scene::Simplify() noexcept
{
    auto& mesh = scene_objects_[active_scene_object_].mesh;
    mesh = mesh::Simplify(mesh, 0.5f);
}


void Scene::Render(gfx::DrawMode draw_mode)
{

	auto view_transform = camera_.GetViewTansform();
	UpdateProjectionTransform();

	for (const auto& [mesh, material] : scene_objects_) {

		const auto view_model_transform = view_transform * mesh.GetModelTransform();
		shader_program_.SetUniform("view_model_transform", view_model_transform);

		// generally, normals should be transformed by the upper 3x3 inverse transpose of the view-model matrix, however,
		// this is unnecessary in this context because meshes are only transformed by rotations and translations (which are
		// orthogonal matrices and therefore the inverse transpose of the view-model matrix is to view-model matrix itself)
		// in addition to uniform scaling (which is undone when the transformed normal is renormalized in the vertex shader)
		shader_program_.SetUniform("normal_transform", mat3{view_model_transform});

		shader_program_.SetUniform("material.ambient", material.ambient());
		shader_program_.SetUniform("material.diffuse", material.diffuse());
		shader_program_.SetUniform("material.specular", material.specular());
		shader_program_.SetUniform("material.shininess", material.shininess() * 128.f);

		mesh.Draw(draw_mode);
	}
}

void Scene::UpdateProjectionTransform() 
{
    static std::pair<int, int> prev_window_dimensions;
	const auto window_dimensions = window_.GetSize();
	const auto width = window_dimensions.first;
	const auto height = window_dimensions.second;

	if ( width && height && window_dimensions != prev_window_dimensions) {
		const auto [field_of_view_y, z_near, z_far] = kViewFrustrum;
		const auto aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
		const auto projection_transform = glm::perspective(field_of_view_y, aspect_ratio, z_near, z_far);
		shader_program_.SetUniform("projection_transform", projection_transform);
		prev_window_dimensions = window_dimensions;
	}
}

void Scene::HandleMouseButtonClick(int button, int action, int mods)
{
	camera_.ProcessMouseButtonClick(button, action, mods);
}

void Scene::HandleMouseMove(double mouse_x, double mouse_y)
{
	auto [width, height] = window_.GetSize();
	camera_.ProcessMouseMove(mouse_x, mouse_y, width, height);
}

void Scene::HandleKeyPress(const int key_code) {

    const auto scene_objects_size = static_cast<int>(scene_objects_.size());
    if (!scene_objects_size) return;

    switch (key_code) {
    case GLFW_KEY_S: {
        auto& mesh = scene_objects_[active_scene_object_].mesh;
        mesh = mesh::Simplify(mesh, 0.5f);
        break;
    }
    case GLFW_KEY_P: {
        static auto use_phong_shading = false;
        use_phong_shading = !use_phong_shading;
        shader_program_.SetUniform("use_phong_shading", use_phong_shading);
        break;
    }
    case GLFW_KEY_N:
        if (++active_scene_object_ >= scene_objects_size) {
            active_scene_object_ = 0;
        }
        break;
    case GLFW_KEY_B:
        if (--active_scene_object_ < 0) {
            active_scene_object_ = scene_objects_size - 1;
        }
        break;
    default:
        break;
    }
}

void Scene::HandleWindowResize(int width, int height)
{
    UpdateProjectionTransform();
}

