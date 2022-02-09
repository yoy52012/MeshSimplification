#pragma once

#include <string_view>

namespace gfx 
{
class Mesh;

    namespace obj_loader 
    {
        /**
         * \brief Loads a triangle mesh from an .obj file.
         * \param filepath The filepath to the .obj file.
         * \return A mesh defined by the position, texture coordinates, normals, and indices specified in the .obj file.
         * \throw std::runtime_error Indicates the file cannot be opened or the file load failed.
         */
        Mesh LoadMesh(const std::string_view filepath);
    }
}
