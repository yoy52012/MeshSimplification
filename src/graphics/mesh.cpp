#include "graphics/mesh.h"

#include <stdexcept>
#include <utility>

#include <GLFW/glfw3.h>

using namespace gfx;
using namespace glm;
using namespace std;

namespace {

/**
 * \brief Ensures the provided vertex positions, texture coordinates, normals, and element indices describe a
 *        triangle mesh in addition to enforcing alignment between vertex attribute.
 * \throw invalid_argument Indicates the provided arguments do not represent a valid triangle mesh.
 */
void Validate(
	const vector<vec3>& positions,
	const vector<vec2>& texture_coordinates,
	const vector<vec3>& normals,
	const vector<GLuint>& indices) {

	if (positions.empty()) {
		throw invalid_argument{"Vertex positions must be specified"};
	}
	if (indices.empty() && positions.size() % 3 != 0 || indices.size() % 3 != 0) {
		throw invalid_argument{"Object must be a triangle mesh"};
	}
	if (indices.empty() && !texture_coordinates.empty() && positions.size() != texture_coordinates.size()) {
		throw invalid_argument{"Texture coordinates must align with position data"};
	}
	if (indices.empty() && !normals.empty() && positions.size() != normals.size()) {
		throw invalid_argument{"Vertex normals must align with position data"};
	}
}
}

Mesh::Mesh(
    std::vector<glm::vec3> positions,
    std::vector<glm::vec2> texture_coordinates,
    std::vector<glm::vec3> normals,
	std::vector<unsigned int> indices,
	mat4 model_transform,
    glm::vec3 bmin,
    glm::vec3 bmax)
	: positions_{move(positions)},
	  texture_coordinates_{move(texture_coordinates)},
	  normals_{move(normals)},
	  indices_{move(indices)},
	  model_transform_{move(model_transform)},
	  bmin_ (std::move(bmin)),
	  bmax_ (std::move(bmax))
{

	Validate(positions_, texture_coordinates_, normals_, indices_);

	glGenVertexArrays(1, &vertex_array_);
	glBindVertexArray(vertex_array_);

	glGenBuffers(1, &vertex_buffer_);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);

	// allocate memory for the vertex buffer
	const auto positions_size = sizeof(vec3) * positions_.size();
	const auto texture_coordinates_size = sizeof(vec2) * texture_coordinates_.size();
	const auto normals_size = sizeof(vec3) * normals_.size();
	const auto buffer_size = positions_size + texture_coordinates_size + normals_size;
	glBufferData(GL_ARRAY_BUFFER, buffer_size, nullptr, GL_STATIC_DRAW);

	// copy positions to the vertex buffer
	glBufferSubData(GL_ARRAY_BUFFER, 0, positions_size, positions_.data());
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid*>(0));
	glEnableVertexAttribArray(0);

	// copy texture coordinates to the vertex buffer
	if (!texture_coordinates_.empty()) {
		glBufferSubData(GL_ARRAY_BUFFER, positions_size, texture_coordinates_size, texture_coordinates_.data());
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid*>(positions_size));
		glEnableVertexAttribArray(1);
	}

	// copy normals to the vertex buffer
	if (!normals_.empty()) {
		const size_t offset = positions_size + texture_coordinates_size;
		glBufferSubData(GL_ARRAY_BUFFER, offset, normals_size, normals_.data());
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<GLvoid*>(offset));
		glEnableVertexAttribArray(2);
	}

	// copy indices to the element buffer
	if (!indices_.empty()) {
		const size_t indices_size = sizeof(GLuint) * indices_.size();
		glGenBuffers(1, &element_buffer_);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size, indices_.data(), GL_STATIC_DRAW);
	}
}

Mesh::~Mesh() {
	glDeleteVertexArrays(1, &vertex_array_);
	glDeleteBuffers(1, &vertex_buffer_);
	glDeleteBuffers(1, &element_buffer_);
}

Mesh::Mesh(Mesh&& mesh) noexcept {
	*this = move(mesh);
}

Mesh& Mesh::operator=(Mesh&& mesh) noexcept {

	if (this == &mesh) return *this;

	glDeleteVertexArrays(1, &vertex_array_);
	glDeleteBuffers(1, &vertex_buffer_);
	glDeleteBuffers(1, &element_buffer_);

	vertex_array_ = mesh.vertex_array_;
	vertex_buffer_ = mesh.vertex_buffer_;
	element_buffer_ = mesh.element_buffer_;

	mesh.vertex_array_ = mesh.vertex_buffer_ = mesh.element_buffer_ = 0u;

	positions_ = move(mesh.positions_);
	texture_coordinates_ = move(mesh.texture_coordinates_);
	normals_ = move(mesh.normals_);
	indices_ = move(mesh.indices_);
	model_transform_ = move(mesh.model_transform_);

	return *this;
}
