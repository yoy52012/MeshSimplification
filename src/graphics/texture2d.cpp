#include "graphics/texture2d.h"

#include <format>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace gfx;
using namespace std;

namespace {

/** \brief Gets the maximum number of texture units allowed by the host GPU. */
GLint GetMaxTextureUnits() noexcept {
	static GLint max_texture_units = 0;
	if (!max_texture_units) {
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &max_texture_units);
	}
	return max_texture_units;
}
}

Texture2d::Texture2d(const string_view filepath, const int texture_unit_index)
	: texture_unit_index_{texture_unit_index} {

	const auto max_texture_units = GetMaxTextureUnits();
	if ( texture_unit_index >= max_texture_units) {
		throw std::out_of_range{ std::string(texture_unit_index + " exceeds maximum texture unit index " + max_texture_units - 1)};
	}

	glActiveTexture(GL_TEXTURE0 + texture_unit_index_);
	glGenTextures(1, &id_);
	glBindTexture(GL_TEXTURE_2D, id_);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);
	if (int width, height, channels; auto* data = stbi_load(filepath.data(), &width, &height, &channels, 0)) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(data);
	} else {
		throw std::runtime_error{std::string("Unable to open " + std::string(filepath))};
	}
}
