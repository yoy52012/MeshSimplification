#pragma once

#include <stdexcept>

#include <glm/vec3.hpp>

namespace gfx {

/**
 * \brief An enumeration for various material types.
 * \see Material::FromType
 */
	enum class MaterialType {
		Brass,
		Bronze,
		Chrome,
		Copper,
		Emerald,
		Gold,
		Jade,
		Obsidian,
		Pearl,
		Ruby,
		Silver,
		Turquoise,
		Count
	};

    constexpr const char* MaterialTypeToString(MaterialType mtl) throw()
    {
        switch (mtl)
        {
        case gfx::MaterialType::Brass:
            return "Brass";
        case gfx::MaterialType::Bronze:
            return "Bronze";
        case gfx::MaterialType::Chrome:
            return "Chrome";
        case gfx::MaterialType::Copper:
            return "Copper";
        case gfx::MaterialType::Emerald:
            return "Emerald";
        case gfx::MaterialType::Gold:
            return "Gold";
        case gfx::MaterialType::Jade:
            return "Jade";
        case gfx::MaterialType::Obsidian:
            return "Obsidian";
        case gfx::MaterialType::Pearl:
            return "Pearl";
        case gfx::MaterialType::Ruby:
            return "Ruby";
        case gfx::MaterialType::Silver:
            return "Silver";
        case gfx::MaterialType::Turquoise:
            return "Turquoise";
        default:
            throw std::invalid_argument("Unimplemented material type");
        }
    }

	constexpr const char* MaterialTypeArray[static_cast<int>(MaterialType::Count)] = { 
		MaterialTypeToString(MaterialType::Brass),
		MaterialTypeToString(MaterialType::Bronze),
		MaterialTypeToString(MaterialType::Chrome),
        MaterialTypeToString(MaterialType::Copper),
        MaterialTypeToString(MaterialType::Emerald),
        MaterialTypeToString(MaterialType::Gold),
        MaterialTypeToString(MaterialType::Jade),
        MaterialTypeToString(MaterialType::Obsidian),
        MaterialTypeToString(MaterialType::Pearl),
        MaterialTypeToString(MaterialType::Ruby),
        MaterialTypeToString(MaterialType::Silver),
        MaterialTypeToString(MaterialType::Turquoise) 
	};


/** \brief A type of material with light reflectance properties. */
class Material {

public:
	/**
	 * \brief A static factory function to construct a material from a dedicated material type.
	 * \param material_type The material type representing the material to construct.
	 * \return A material containing light reflectance properties for the provided material type.
	 */
	static Material FromType(const MaterialType material_type) {
		switch (material_type) {
			case MaterialType::Brass: {
                const glm::vec3 kAmbient{ 0.329412f, 0.223529f, 0.027451f };
                const glm::vec3 kDiffuse{ .780392f, .568627f, .113725f };
                const glm::vec3 kSpecular{ .992157f, .941176f, .807843f };
                const float kShininess = .21794872f;
				return Material{kAmbient, kDiffuse, kSpecular, kShininess};
			}
			case MaterialType::Bronze: {
                const glm::vec3 kAmbient{ .2125f, .1275f, .054f };
                const glm::vec3 kDiffuse{ .714f, .4284f, .18144f };
                const glm::vec3 kSpecular{ .393548f, .271906f, .166721f };
                const float kShininess = .2f;
				return Material{kAmbient, kDiffuse, kSpecular, kShininess};
			};
			case MaterialType::Chrome: {
				const glm::vec3 kAmbient{.25f, .25f, .25f};
				const glm::vec3 kDiffuse{.4f, .4f, .4f};
				const glm::vec3 kSpecular{.774597f, .774597f, .774597f};
				const float kShininess = .6f;
				return Material{kAmbient, kDiffuse, kSpecular, kShininess};
			}
			case MaterialType::Copper: {
				const glm::vec3 kAmbient{.19125f, .0735f, .0225f};
				const glm::vec3 kDiffuse{.7038f, .27048f, .0828f};
				const glm::vec3 kSpecular{.256777f, .137622f, .086014f};
				const float kShininess = .1f;
				return Material{kAmbient, kDiffuse, kSpecular, kShininess};
			}
			case MaterialType::Emerald: {
				const glm::vec3 kAmbient{.0215f, .1745f, .0215f};
				const glm::vec3 kDiffuse{.07568f, .61424f, .07568f};
				const glm::vec3 kSpecular{.633f, .727811f, .633f};
				const float kShininess = .6f;
				return Material{kAmbient, kDiffuse, kSpecular, kShininess};
			}
			case MaterialType::Gold: {
				const glm::vec3 kAmbient{.24725f, .1995f, .0745f};
				const glm::vec3 kDiffuse{.75164f, .60648f, .22648f};
				const glm::vec3 kSpecular{.628281f, .555802f, .366065f};
				const float kShininess = .4f;
				return Material{kAmbient, kDiffuse, kSpecular, kShininess};
			}
			case MaterialType::Jade: {
                const glm::vec3 kAmbient{ .135f, .2225f, .1575f };
                const glm::vec3 kDiffuse{ .54f, .89f, .63f };
                const glm::vec3 kSpecular{ .316228f, .316228f, .316228f };
                const float kShininess = .1f;
				return Material{kAmbient, kDiffuse, kSpecular, kShininess};
			}
			case MaterialType::Obsidian: {
                const glm::vec3 kAmbient{ .05375f, .05f, .06625f };
                const glm::vec3 kDiffuse{ .18275f, .17f, .22525f };
                const glm::vec3 kSpecular{ .332741f, .328634f, .346435f };
                const float kShininess = .3f;
				return Material{kAmbient, kDiffuse, kSpecular, kShininess};
			}
			case MaterialType::Pearl: {
                const glm::vec3 kAmbient{ .25f, .20725f, .20725f };
                const glm::vec3 kDiffuse{ 1.f, .829f, .829f };
                const glm::vec3 kSpecular{ .296648f, .296648f, .296648f };
				const float kShininess = .088f;
				return Material{kAmbient, kDiffuse, kSpecular, kShininess};
			}
			case MaterialType::Ruby: {
				const glm::vec3 kAmbient{.1745f, .01175f, .01175f};
				const glm::vec3 kDiffuse{.61424f, .04136f, .04136f};
				const glm::vec3 kSpecular{.727811f, .626959f, .626959f};
				const float kShininess = .6f;
				return Material{kAmbient, kDiffuse, kSpecular, kShininess};
			}
			case MaterialType::Silver: {
                const glm::vec3 kAmbient{ .19225f, .19225f, .19225f };
                const glm::vec3 kDiffuse{ .50754f, .50754f, .50754f };
                const glm::vec3 kSpecular{ .508273f, .508273f, .508273f };
                const float kShininess = .4f;
				return Material{kAmbient, kDiffuse, kSpecular, kShininess};
			}
			case MaterialType::Turquoise: {
                const glm::vec3 kAmbient{ .1f, .18725f, .1745f };
                const glm::vec3 kDiffuse{ .396f, .74151f, .69102f };
                const glm::vec3 kSpecular{ .297254f, .30829f, .306678f };
                const float kShininess = .1f;
				return Material{kAmbient, kDiffuse, kSpecular, kShininess};
			}
			default:
				throw std::invalid_argument{"Invalid material type"};
		}
	}

	/**
	 * \brief Initializes a material.
	 * \param ambient The ambient light reflected.
	 * \param diffuse The diffuse light reflected.
	 * \param specular The specular light reflected.
	 * \param shininess The degree of shininess reflected from specular highlights.
	 */
	constexpr Material(
		const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const float shininess)
		: ambient_{ambient},
		  diffuse_{diffuse},
		  specular_{specular},
		  shininess_{shininess} {}

	/** \brief Gets the ambient light reflected. */
	[[nodiscard]]  glm::vec3 ambient() const { return ambient_; }

	/** \brief Gets the diffuse light reflected. */
	[[nodiscard]]  glm::vec3 diffuse() const { return diffuse_; }

	/** \brief Gets the specular light reflected. */
	[[nodiscard]]  glm::vec3 specular() const { return specular_; }

	/** \brief Gets the  degree of shininess reflected from specular highlights. */
	[[nodiscard]]  float shininess() const { return shininess_; }

private:
	glm::vec3 ambient_;
	glm::vec3 diffuse_;
	glm::vec3 specular_;
	float shininess_;
};
}
