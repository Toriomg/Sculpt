#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "MeshLoader.hpp"
#include "Platform/Graphics/Vertex.hpp"
#include "Renderer/Mesh.hpp"

std::shared_ptr<Mesh> MeshLoader::Load(std::string const& filepath) {
    Assimp::Importer importer;
    // aiProcess_FlipUVs: OpenGL places V=0 at the bottom, but most DCC tools export V=0 at the top.
    // aiProcess_CalcTangentSpace: pre-computed for future normal mapping; not consumed by the
    // current Vertex struct.
    aiScene const* scene = importer.ReadFile(filepath, aiProcess_Triangulate |
                                                           aiProcess_GenSmoothNormals |
                                                           aiProcess_FlipUVs |
                                                           aiProcess_CalcTangentSpace);

    if ((scene == nullptr) ||
        ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0u) ||
        (scene->mRootNode == nullptr))
    {
        CORE_LOG_ERROR("[MeshLoader] Assimp error loading {0}: {1}", filepath,
                       importer.GetErrorString());
        return nullptr;
    }

    if (scene->mNumMeshes == 0) {
        CORE_LOG_ERROR("[MeshLoader] No meshes found in file: {0}", filepath);
        return nullptr;
    }

    // Only the first mesh in the file is imported; multi-mesh files are not supported yet.
    aiMesh const* mesh = scene->mMeshes[0];

    std::vector<Vertex> vertices;
    vertices.reserve(mesh->mNumVertices);

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;

        vertex.pos.x = mesh->mVertices[i].x;
        vertex.pos.y = mesh->mVertices[i].y;
        vertex.pos.z = mesh->mVertices[i].z;

        if (mesh->HasNormals()) {
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        } else {
            vertex.normal = {0.0f, 0.0f, 0.0f};
        }

        if (mesh->mTextureCoords[0] != nullptr) {
            vertex.texCoord.x = mesh->mTextureCoords[0][i].x;
            vertex.texCoord.y = mesh->mTextureCoords[0][i].y;
        } else {
            vertex.texCoord = {0.0f, 0.0f};
        }

        vertices.push_back(vertex);
    }

    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace const face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) { indices.push_back(face.mIndices[j]); }
    }

    if (vertices.empty() || indices.empty()) {
        CORE_LOG_WARN("[MeshLoader] Model loading resulted in an empty mesh for file: {0}",
                      filepath);
        return nullptr;
    }

    CORE_LOG_INFO("[MeshLoader] Successfully loaded model data: {0} ({1} vertices, {2} indices)",
                  filepath, vertices.size(), indices.size());

    return Mesh::CreateMeshFromData(vertices.data(), vertices.size() * sizeof(Vertex),
                                    indices.data(), indices.size());
}
