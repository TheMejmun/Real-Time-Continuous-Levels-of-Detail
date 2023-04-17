//
// Created by Sam on 2023-04-07.
//

#include <fstream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "importer.h"
#include "printer.h"

std::vector<char> Importinator::readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        THROW("Failed to open file " + filename);
    }

    auto fileSize = file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}

Importinator::Mesh Importinator::importMesh(const std::string &filename) {
    Importinator::Mesh out{};
    Assimp::Importer importer{};

    const aiScene *scene = importer.ReadFile(filename,
                                             aiProcess_CalcTangentSpace |
                                             aiProcess_Triangulate |
                                             aiProcess_JoinIdenticalVertices |
                                             aiProcess_SortByPType |
                                             aiProcess_GenNormals |
                                             aiProcess_GenUVCoords |
                                             aiProcess_OptimizeGraph |
                                             aiProcess_OptimizeMeshes |
                                             aiProcess_FlipUVs);

    if (scene == nullptr) {
        THROW(importer.GetErrorString());
    } else {
        DBG "Read mesh " << filename ENDL;
    }

    for (uint32_t meshIndex = 0; meshIndex < (scene->mNumMeshes); ++meshIndex) {
        const aiMesh *mesh = *(scene->mMeshes + meshIndex);

        out.vertices.resize(out.vertices.size() + mesh->mNumVertices);

        for (uint32_t i = 0; i < (mesh->mNumVertices); ++i) {
            if (mesh->HasPositions()) {
                aiVector3D vertex = *(mesh->mVertices + i);
                out.vertices[i].pos = glm::vec3(vertex.x, vertex.y, vertex.z);
            }

            if (mesh->HasNormals()) {
                aiVector3D normal = *(mesh->mNormals + i);
                out.vertices[i].normal = glm::vec3(normal.x, normal.y, normal.z);
            }

            if (mesh->HasTangentsAndBitangents()) {
                aiVector3D tangent = *(mesh->mTangents + i);
                aiVector3D bitangent = *(mesh->mBitangents + i);

                out.vertices[i].tangent = glm::vec3(tangent.x, tangent.y, tangent.z);
                out.vertices[i].bitangent = glm::vec3(bitangent.x, bitangent.y, bitangent.z);
            }

            if (mesh->HasTextureCoords(0)) {
                // A vertex can have multiple UVs. We'll just pick the first one. TODO
                aiVector3D uv = *(mesh->mTextureCoords[0] + i);
                out.vertices[i].uvw = glm::vec3(uv.x, uv.y, uv.z);
            }
        }

        out.indices.resize(out.indices.size() + (mesh->mNumFaces * 3));

        for (uint32_t i = 0; i < (mesh->mNumFaces); ++i) {
            aiFace face = *(mesh->mFaces + i);
            out.indices[i * 3] = (*(face.mIndices));
            out.indices[i * 3 + 1] = (*(face.mIndices + 1));
            out.indices[i * 3 + 2] = (*(face.mIndices + 2));
        }
    }
    DBG "\tIndices: " << out.indices.size() ENDL;
    DBG "\tVertices: " << out.vertices.size() ENDL;

    return out;
}