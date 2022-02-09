#include "graphics/obj_loader.h"

#include <algorithm>
#include <array>
#include <charconv>
#include <format>
#include <fstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <iostream>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/gtx/hash.hpp>

#include <tiny_obj_loader.h>

#include "graphics/mesh.h"

using namespace gfx;
using namespace glm;
using namespace std;

namespace {

// sentinel value indicating an unspecified face index
constexpr int kInvalidFaceElementIndex = -1;

/**
 * \brief Removes a set of characters from the beginning and end of the string.
 * \param line The string to evaluate.
 * \param delimiter A set of characters (in any order) to remove from the beginning and end of the string.
 * \return A view of the characters in \p delimiter removed from the beginning and end of \p line.
 */
constexpr string_view Trim(string_view line, const string_view delimiter = " \t") noexcept {
	line.remove_prefix(std::min<>(line.find_first_not_of(delimiter), line.size()));
	line.remove_suffix(line.size() - line.find_last_not_of(delimiter) - 1);
	return line;
}

/**
 * \brief Gets tokens delimited by a set of characters.
 * \param line The string to evaluate.
 * \param delimiter The set of characters (in any order) to split the string on.
 * \return A vector of tokens in \p line split on the characters in \p delimiter.
 */
vector<string_view> Split(const string_view line, const string_view delimiter = " \t") {
	vector<string_view> tokens;
	for (auto i = line.find_first_not_of(delimiter); i < line.size();) {
		const auto j = std::min<>(line.find_first_of(delimiter, i), line.size());
		tokens.push_back(line.substr(i, j - i));
		i = line.find_first_not_of(delimiter, j);
	}
	return tokens;
}

/**
 * \brief Parses a string token.
 * \tparam T The type to convert to.
 * \param token The token to parse.
 * \return The converted value of \p token to type \p T.
 * \throw invalid_argument Indicates the string conversion failed.
 */
template <typename T>
T ParseToken(const string_view token) {
	T value;
	if (const auto [_, error_code] = from_chars(token.data(), token.data() + token.size(), value);
		error_code == errc::invalid_argument) {
		throw invalid_argument{format("Unable to convert {} to type {}", token, typeid(T).name())};
	}
	return value;
}

/**
 * \brief Parses a line in an .obj file.
 * \tparam T The type to convert to.
 * \tparam N The number of items to convert (does not include the first token identifying the line type).
 * \param line The line to parse.
 * \return A vector of size \p N containing each item in \p line converted to type \p T.
 * \throw invalid_argument Indicates if the line format is unsupported.
 */
template <typename C, typename T, int N>
C ParseLine(const string_view line) {
	if (const auto tokens = Split(line); tokens.size() == N + 1) {
		C vec{};
		for (auto i = 1; i <= N; ++i) {
			vec[i - 1] = ParseToken<T>(tokens[i]);
		}
		return vec;
	}
	throw invalid_argument{format("Unsupported format {}", line)};
}



/**
 * \brief Parses a token representing a face element index group.
 * \param token The token to parse. May optionally contain texture coordinate and normal indices.
 * \return A vector containing vertex position, texture coordinate, and normal indices. Unspecified texture
 *         coordinate and normal values are indicated by the value \c npos_index.
 * \throw invalid_argument Indicates the index group format is unsupported.
 */
ivec3 ParseIndexGroup(const string_view token) {
	static constexpr auto kIndexDelimiter = "/";
	const auto tokens = Split(token, kIndexDelimiter);

	switch (ranges::count(token, *kIndexDelimiter)) {
		case 0:
			if (tokens.size() == 1) {
				const auto x = ParseToken<int>(tokens[0]) - 1;
				return {x, kInvalidFaceElementIndex, kInvalidFaceElementIndex};
			}
			break;
		case 1:
			if (tokens.size() == 2) {
				const auto x = ParseToken<int>(tokens[0]) - 1;
				const auto y = ParseToken<int>(tokens[1]) - 1;
				return {x, y, kInvalidFaceElementIndex};
			}
			break;
		case 2:
			if (tokens.size() == 2 && *token.cbegin() != '/' && *(token.cend() - 1) != '/') {
				const auto x = ParseToken<int>(tokens[0]) - 1;
				const auto z = ParseToken<int>(tokens[1]) - 1;
				return {x, kInvalidFaceElementIndex, z};
			}
			if (tokens.size() == 3) {
				const auto x = ParseToken<int>(tokens[0]) - 1;
				const auto y = ParseToken<int>(tokens[1]) - 1;
				const auto z = ParseToken<int>(tokens[2]) - 1;
				return {x, y, z};
			}
			break;
	}

	throw invalid_argument{std::string("Unsupported format " + std::string(token))};
}

/**
 * \brief Parses a line representing a triangular face element.
 * \param line The line to parse.
 * \return An array containing three parsed index groups for the face.
 * \throw invalid_argument Indicates the line format is unsupported.
 */
array<ivec3, 3> ParseFace(const string_view line) {
	if (const auto tokens = Split(line, " \t"); tokens.size() == 4) {
		return {ParseIndexGroup(tokens[1]), ParseIndexGroup(tokens[2]), ParseIndexGroup(tokens[3])};
	}
	throw invalid_argument{std::string("Unsupported format " + std::string(line))};
}

/**
 * \brief Loads a triangle mesh from an input stream representing the contents of an .obj file.
 * \param is The input stream to parse.
 * \return A mesh defined by the position, texture coordinates, normals, and indices specified in the input stream.
 * \throw invalid_argument Indicates the input stream contains an unsupported format.
 */
Mesh LoadMesh(istream& is) {
	vector<vec3> positions;
	vector<vec2> texture_coordinates;
	vector<vec3> normals;
	vector<array<ivec3, 3>> faces;

	for (string line; getline(is, line);) {
		if (const auto line_view = Trim(line); !line_view.empty() && !line_view.starts_with('#')) {
			if (line_view.starts_with("v ")) {
				positions.push_back(ParseLine<vec3,float, 3>(line_view));
			} else if (line_view.starts_with("vt ")) {
				texture_coordinates.push_back(ParseLine<vec2, float, 2>(line_view));
			} else if (line_view.starts_with("vn ")) {
				normals.push_back(ParseLine<vec3, float, 3>(line_view));
			} else if (line_view.starts_with("f ")) {
				faces.push_back(ParseFace(line_view));
			}
		}
	}

	if (faces.empty()) return Mesh{positions, texture_coordinates, normals};

	vector<vec3> ordered_positions;
	vector<vec2> ordered_texture_coordinates;
	vector<vec3> ordered_normals;
	vector<GLuint> indices;
	indices.reserve(faces.size() * 3);

	// For each index group, store texture coordinate and normals at the same index as the vertex position so that
	// data is aligned when sent to the vertex shader. Occasionally, index groups may specify different texture
	// coordinates or normals for the same vertex position. To handle this situation, an unordered map is used to
	// keep track of unique index groups and appends new position, texture coordinate, and normal triples to the end
	// of each respective ordered array as necessary.
	for (std::unordered_map<ivec3, GLuint> unique_index_groups; const auto& face : faces) {
		for (const auto& index_group : face) {
			if (const auto iterator = unique_index_groups.find(index_group); iterator == unique_index_groups.end()) {
				const auto position_index = index_group[0];
				ordered_positions.push_back(positions.at(position_index));
				if (const auto texture_coordinate_index = index_group[1]; texture_coordinate_index != kInvalidFaceElementIndex) {
					ordered_texture_coordinates.push_back(texture_coordinates.at(texture_coordinate_index));
				}
				if (const auto normal_index = index_group[2]; normal_index != kInvalidFaceElementIndex) {
					ordered_normals.push_back(normals.at(normal_index));
				}
				const auto index = static_cast<GLuint>(ordered_positions.size()) - 1u;
				indices.push_back(index);
				unique_index_groups.emplace(index_group, index);
			} else {
				indices.push_back(iterator->second);
			}
		}
	}

	return Mesh{ordered_positions, ordered_texture_coordinates, ordered_normals, indices};
}
}

Mesh obj_loader::LoadMesh(const string_view filepath) 
{
	if (ifstream ifs{filepath.data()}; ifs.good()) {
		return ::LoadMesh(ifs);
	}
	throw runtime_error{std::string("Unable to open " + std::string(filepath))};
}

static bool HasSmoothingGroup(const tinyobj::shape_t& shape)
{
    for (size_t i = 0; i < shape.mesh.smoothing_group_ids.size(); i++) {
        if (shape.mesh.smoothing_group_ids[i] > 0) {
            return true;
        }
    }
    return false;
}

static void CalcNormal(glm::vec3& normal, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) 
{
    glm::vec3 v10 = v1 - v0;
    glm::vec3 v20 = v2 - v0;

    normal[0] = v10[1] * v20[2] - v10[2] * v20[1];
    normal[1] = v10[2] * v20[0] - v10[0] * v20[2];
    normal[2] = v10[0] * v20[1] - v10[1] * v20[0];

    float len2 = normal[0] * normal[0] + normal[1] * normal[1] + normal[2] * normal[2];
    if (len2 > 0.0f) {
        float len = sqrtf(len2);

        normal[0] /= len;
        normal[1] /= len;
        normal[2] /= len;
    }
}

static void ComputeSmoothingNormals(const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape, std::map<int, vec3>& smoothVertexNormals) 
{
    smoothVertexNormals.clear();
    std::map<int, vec3>::iterator iter;

    for (size_t f = 0; f < shape.mesh.indices.size() / 3; f++) {
        // Get the three indexes of the face (all faces are triangular)
        tinyobj::index_t idx0 = shape.mesh.indices[3 * f + 0];
        tinyobj::index_t idx1 = shape.mesh.indices[3 * f + 1];
        tinyobj::index_t idx2 = shape.mesh.indices[3 * f + 2];

        // Get the three vertex indexes and coordinates
        int vi[3];      // indexes
        std::array<glm::vec3, 3> v;  // coordinates

        for (int k = 0; k < 3; k++) {
            vi[0] = idx0.vertex_index;
            vi[1] = idx1.vertex_index;
            vi[2] = idx2.vertex_index;
            assert(vi[0] >= 0);
            assert(vi[1] >= 0);
            assert(vi[2] >= 0);

            v[0][k] = attrib.vertices[3 * vi[0] + k];
            v[1][k] = attrib.vertices[3 * vi[1] + k];
            v[2][k] = attrib.vertices[3 * vi[2] + k];
        }

        // Compute the normal of the face
        glm::vec3 normal;
        CalcNormal(normal, v[0], v[1], v[2]);

        // Add the normal to the three vertexes
        for (size_t i = 0; i < 3; ++i) {
            iter = smoothVertexNormals.find(vi[i]);
            if (iter != smoothVertexNormals.end()) {
                // add
                iter->second[0] += normal[0];
                iter->second[1] += normal[1];
                iter->second[2] += normal[2];
            }
            else {
                smoothVertexNormals[vi[i]][1] = normal[1];
                smoothVertexNormals[vi[i]][2] = normal[2];
                smoothVertexNormals[vi[i]][0] = normal[0];
            }
        }

    }  // f

    // Normalize the normals, that is, make them unit vectors
    for (iter = smoothVertexNormals.begin(); iter != smoothVertexNormals.end();
        iter++) {
        iter->second = glm::normalize(iter->second);
    }

}

static void ComputeAllSmoothingNormals(tinyobj::attrib_t& attrib, std::vector<tinyobj::shape_t>& shapes) {
    glm::vec3 p[3];
    for (size_t s = 0, slen = shapes.size(); s < slen; ++s) {
        const tinyobj::shape_t& shape(shapes[s]);
        size_t facecount = shape.mesh.num_face_vertices.size();
        assert(shape.mesh.smoothing_group_ids.size());

        for (size_t f = 0, flen = facecount; f < flen; ++f) {
            for (unsigned int v = 0; v < 3; ++v) {
                tinyobj::index_t idx = shape.mesh.indices[3 * f + v];
                assert(idx.vertex_index != -1);
                p[v][0] = attrib.vertices[3 * idx.vertex_index];
                p[v][1] = attrib.vertices[3 * idx.vertex_index + 1];
                p[v][2] = attrib.vertices[3 * idx.vertex_index + 2];
            }

            // cross(p[1] - p[0], p[2] - p[0])
            float nx = (p[1][1] - p[0][1]) * (p[2][2] - p[0][2]) -
                (p[1][2] - p[0][2]) * (p[2][1] - p[0][1]);
            float ny = (p[1][2] - p[0][2]) * (p[2][0] - p[0][0]) -
                (p[1][0] - p[0][0]) * (p[2][2] - p[0][2]);
            float nz = (p[1][0] - p[0][0]) * (p[2][1] - p[0][1]) -
                (p[1][1] - p[0][1]) * (p[2][0] - p[0][0]);

            // Don't normalize here.
            for (unsigned int v = 0; v < 3; ++v) {
                tinyobj::index_t idx = shape.mesh.indices[3 * f + v];
                attrib.normals[3 * idx.normal_index] += nx;
                attrib.normals[3 * idx.normal_index + 1] += ny;
                attrib.normals[3 * idx.normal_index + 2] += nz;
            }
        }
    }

    assert(attrib.normals.size() % 3 == 0);
    for (size_t i = 0, nlen = attrib.normals.size() / 3; i < nlen; ++i) {
        tinyobj::real_t& nx = attrib.normals[3 * i];
        tinyobj::real_t& ny = attrib.normals[3 * i + 1];
        tinyobj::real_t& nz = attrib.normals[3 * i + 2];
        tinyobj::real_t len = sqrtf(nx * nx + ny * ny + nz * nz);
        tinyobj::real_t scale = len == 0 ? 0 : 1 / len;
        nx *= scale;
        ny *= scale;
        nz *= scale;
    }
}

static void ComputeSmoothingShape(tinyobj::attrib_t& inattrib, tinyobj::shape_t& inshape,
    std::vector<std::pair<unsigned int, unsigned int>>& sortedids,
    unsigned int idbegin, unsigned int idend,
    std::vector<tinyobj::shape_t>& outshapes,
    tinyobj::attrib_t& outattrib) {
    unsigned int sgroupid = sortedids[idbegin].first;
    bool hasmaterials = inshape.mesh.material_ids.size();
    // Make a new shape from the set of faces in the range [idbegin, idend).
    outshapes.emplace_back();
    tinyobj::shape_t& outshape = outshapes.back();
    outshape.name = inshape.name;
    // Skip lines and points.

    std::unordered_map<unsigned int, unsigned int> remap;
    for (unsigned int id = idbegin; id < idend; ++id) {
        unsigned int face = sortedids[id].second;

        outshape.mesh.num_face_vertices.push_back(3); // always triangles
        if (hasmaterials)
            outshape.mesh.material_ids.push_back(inshape.mesh.material_ids[face]);
        outshape.mesh.smoothing_group_ids.push_back(sgroupid);
        // Skip tags.

        for (unsigned int v = 0; v < 3; ++v) {
            tinyobj::index_t inidx = inshape.mesh.indices[3 * face + v], outidx;
            assert(inidx.vertex_index != -1);
            auto iter = remap.find(inidx.vertex_index);
            // Smooth group 0 disables smoothing so no shared vertices in that case.
            if (sgroupid && iter != remap.end()) {
                outidx.vertex_index = (*iter).second;
                outidx.normal_index = outidx.vertex_index;
                outidx.texcoord_index = (inidx.texcoord_index == -1) ? -1 : outidx.vertex_index;
            }
            else {
                assert(outattrib.vertices.size() % 3 == 0);
                unsigned int offset = static_cast<unsigned int>(outattrib.vertices.size() / 3);
                outidx.vertex_index = outidx.normal_index = offset;
                outidx.texcoord_index = (inidx.texcoord_index == -1) ? -1 : offset;
                outattrib.vertices.push_back(inattrib.vertices[3 * inidx.vertex_index]);
                outattrib.vertices.push_back(inattrib.vertices[3 * inidx.vertex_index + 1]);
                outattrib.vertices.push_back(inattrib.vertices[3 * inidx.vertex_index + 2]);
                outattrib.normals.push_back(0.0f);
                outattrib.normals.push_back(0.0f);
                outattrib.normals.push_back(0.0f);
                if (inidx.texcoord_index != -1) {
                    outattrib.texcoords.push_back(inattrib.texcoords[2 * inidx.texcoord_index]);
                    outattrib.texcoords.push_back(inattrib.texcoords[2 * inidx.texcoord_index + 1]);
                }
                remap[inidx.vertex_index] = offset;
            }
            outshape.mesh.indices.push_back(outidx);
        }
    }
}

static void ComputeSmoothingShapes(tinyobj::attrib_t& inattrib, std::vector<tinyobj::shape_t>& inshapes, std::vector<tinyobj::shape_t>& outshapes, tinyobj::attrib_t& outattrib) 
{
    for (size_t s = 0, slen = inshapes.size(); s < slen; ++s) {
        tinyobj::shape_t& inshape = inshapes[s];

        unsigned int numfaces = static_cast<unsigned int>(inshape.mesh.smoothing_group_ids.size());
        assert(numfaces);
        std::vector<std::pair<unsigned int, unsigned int>> sortedids(numfaces);
        for (unsigned int i = 0; i < numfaces; ++i)
            sortedids[i] = std::make_pair(inshape.mesh.smoothing_group_ids[i], i);
        sort(sortedids.begin(), sortedids.end());

        unsigned int activeid = sortedids[0].first;
        unsigned int id = activeid, idbegin = 0, idend = 0;
        // Faces are now bundled by smoothing group id, create shapes from these.
        while (idbegin < numfaces) {
            while (activeid == id && ++idend < numfaces)
                id = sortedids[idend].first;
            ComputeSmoothingShape(inattrib, inshape, sortedids, idbegin, idend,
                outshapes, outattrib);
            activeid = id;
            idbegin = idend;
        }
    }
}


Mesh obj_loader::LoadMesh1(const string_view filepath)
{
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> texture_coordinates;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    glm::vec3 bmin(std::numeric_limits<float>::max());
    glm::vec3 bmax(-std::numeric_limits<float>::max());

    tinyobj::attrib_t inattrib;
    std::vector<tinyobj::shape_t> inshapes;
	std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;

	auto GetBaseDir = [](const string_view filepath)-> std::string_view {
		if (filepath.find_last_of("/\\") != std::string::npos)
			return filepath.substr(0, filepath.find_last_of("/\\"));
		return "";
	};

	auto base_dir = GetBaseDir(filepath);
    bool ret = tinyobj::LoadObj(&inattrib, &inshapes, &materials, &warn, &err, std::string(filepath).c_str(), std::string(base_dir).c_str());

    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    if (!ret) {
		throw runtime_error{ std::string("Failed to load " + std::string(filepath)) };
    }

    std::cout << "Num of vertices  = " << (int)(inattrib.vertices.size()) / 3 << std::endl;
    std::cout << "Num of normals   = " << (int)(inattrib.normals.size()) / 3 << std::endl;
    std::cout << "Num of texcoords = " << (int)(inattrib.texcoords.size()) / 2 << std::endl;
    std::cout << "Num of materials = " << (int)materials.size() << std::endl;
    std::cout << "Num of shapes    = " << (int)inshapes.size() << std::endl;

    const bool regen_all_normals = inattrib.normals.size() == 0;
    tinyobj::attrib_t outattrib;
    std::vector<tinyobj::shape_t> outshapes;
    if (regen_all_normals) {
        ComputeSmoothingShapes(inattrib, inshapes, outshapes, outattrib);
        ComputeAllSmoothingNormals(outattrib, outshapes);
    }

    std::vector<tinyobj::shape_t>& shapes = regen_all_normals ? outshapes : inshapes;
    tinyobj::attrib_t& attrib = regen_all_normals ? outattrib : inattrib;

    for (size_t s = 0; s < shapes.size(); s++) {
        
        // Check for smoothing group and compute smoothing normals
        std::map<int, vec3> smoothVertexNormals;
        if (!regen_all_normals && (HasSmoothingGroup(shapes[s]) > 0)) {
            std::cout << "Compute smoothingNormal for shape [" << s << "]" << std::endl;
            ComputeSmoothingNormals(attrib, shapes[s], smoothVertexNormals);
        }

        for (size_t f = 0; f < shapes[s].mesh.indices.size() / 3; f++) {
            tinyobj::index_t idx0 = shapes[s].mesh.indices[3 * f + 0];
            tinyobj::index_t idx1 = shapes[s].mesh.indices[3 * f + 1];
            tinyobj::index_t idx2 = shapes[s].mesh.indices[3 * f + 2];
            
            // Get texturecoords
            std::array<glm::vec2, 3> tc;
            if (attrib.texcoords.size() > 0) {
                if ((idx0.texcoord_index < 0) || (idx1.texcoord_index < 0) || (idx2.texcoord_index < 0)) {
                    // face does not contain valid uv index.
                    tc[0][0] = 0.0f;
                    tc[0][1] = 0.0f;
                    tc[1][0] = 0.0f;
                    tc[1][1] = 0.0f;
                    tc[2][0] = 0.0f;
                    tc[2][1] = 0.0f;
                }
                else {
                    assert(attrib.texcoords.size() > size_t(2 * idx0.texcoord_index + 1));
                    assert(attrib.texcoords.size() > size_t(2 * idx1.texcoord_index + 1));
                    assert(attrib.texcoords.size() > size_t(2 * idx2.texcoord_index + 1));

                    // Flip Y coord.
                    tc[0][0] = attrib.texcoords[2 * idx0.texcoord_index];
                    tc[0][1] = 1.0f - attrib.texcoords[2 * idx0.texcoord_index + 1];
                    tc[1][0] = attrib.texcoords[2 * idx1.texcoord_index];
                    tc[1][1] = 1.0f - attrib.texcoords[2 * idx1.texcoord_index + 1];
                    tc[2][0] = attrib.texcoords[2 * idx2.texcoord_index];
                    tc[2][1] = 1.0f - attrib.texcoords[2 * idx2.texcoord_index + 1];
                }
            }
            else {
                tc[0][0] = 0.0f;
                tc[0][1] = 0.0f;
                tc[1][0] = 0.0f;
                tc[1][1] = 0.0f;
                tc[2][0] = 0.0f;
                tc[2][1] = 0.0f;
            }

            texture_coordinates.push_back(tc[0]);
            texture_coordinates.push_back(tc[1]);
            texture_coordinates.push_back(tc[2]);

            // Get vertex
            std::array<glm::vec3, 3> vertex;
            for (int k = 0; k < 3; k++) {
                int f0 = idx0.vertex_index;
                int f1 = idx1.vertex_index;
                int f2 = idx2.vertex_index;

                assert(f0 >= 0);
                assert(f1 >= 0);
                assert(f2 >= 0);

                vertex[0][k] = attrib.vertices[3 * f0 + k];
                vertex[1][k] = attrib.vertices[3 * f1 + k];
                vertex[2][k] = attrib.vertices[3 * f2 + k];

                bmin[k] = std::min(vertex[0][k], bmin[k]);
                bmin[k] = std::min(vertex[1][k], bmin[k]);
                bmin[k] = std::min(vertex[2][k], bmin[k]);

                bmax[k] = std::max(vertex[0][k], bmax[k]);
                bmax[k] = std::max(vertex[1][k], bmax[k]);
                bmax[k] = std::max(vertex[2][k], bmax[k]);
            }

            indices.push_back(vertices.size() + 0u);
            indices.push_back(vertices.size() + 1u);
            indices.push_back(vertices.size() + 2u);

            vertices.push_back(vertex[0]);
            vertices.push_back(vertex[1]);
            vertices.push_back(vertex[2]);

            // Get normal
            std::array<glm::vec3, 3> normal;
            bool invalid_normal_index = false;
            if (attrib.normals.size() > 0) {
                int nf0 = idx0.normal_index;
                int nf1 = idx1.normal_index;
                int nf2 = idx2.normal_index;

                if ((nf0 < 0) || (nf1 < 0) || (nf2 < 0)) {
                    // normal index is missing from this face.
                    invalid_normal_index = true;
                }
                else {
                    for (int k = 0; k < 3; k++) {
                        assert(size_t(3 * nf0 + k) < attrib.normals.size());
                        assert(size_t(3 * nf1 + k) < attrib.normals.size());
                        assert(size_t(3 * nf2 + k) < attrib.normals.size());
                        normal[0][k] = attrib.normals[3 * nf0 + k];
                        normal[1][k] = attrib.normals[3 * nf1 + k];
                        normal[2][k] = attrib.normals[3 * nf2 + k];
                    }
                }
            }
            else {
                invalid_normal_index = true;
            }

            if (invalid_normal_index && !smoothVertexNormals.empty()) {
                // Use smoothing normals
                int f0 = idx0.vertex_index;
                int f1 = idx1.vertex_index;
                int f2 = idx2.vertex_index;

                if (f0 >= 0 && f1 >= 0 && f2 >= 0) {
                    normal[0][0] = smoothVertexNormals[f0][0];
                    normal[0][1] = smoothVertexNormals[f0][1];
                    normal[0][2] = smoothVertexNormals[f0][2];

                    normal[1][0] = smoothVertexNormals[f1][0];
                    normal[1][1] = smoothVertexNormals[f1][1];
                    normal[1][2] = smoothVertexNormals[f1][2];

                    normal[2][0] = smoothVertexNormals[f2][0];
                    normal[2][1] = smoothVertexNormals[f2][1];
                    normal[2][2] = smoothVertexNormals[f2][2];

                    invalid_normal_index = false;
                }
            }

            if (invalid_normal_index) {
                // compute geometric normal
                CalcNormal(normal[0], vertex[0], vertex[1], vertex[2]);
                normal[1][0] = normal[0][0];
                normal[1][1] = normal[0][1];
                normal[1][2] = normal[0][2];
                normal[2][0] = normal[0][0];
                normal[2][1] = normal[0][1];
                normal[2][2] = normal[0][2];
            }

            normals.push_back(normal[0]);
            normals.push_back(normal[1]);
            normals.push_back(normal[2]);

        }
    }

    return Mesh{ vertices, texture_coordinates, normals, indices, glm::mat4{1.0}, bmin, bmax };
}
