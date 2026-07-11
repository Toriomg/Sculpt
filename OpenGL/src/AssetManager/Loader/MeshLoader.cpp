#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "MeshLoader.h"
#include "Renderer/Mesh.h"
#include "Core/glhead.h"


std::shared_ptr<IAsset> MeshLoader::Load(const std::string& filepath) {
	Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filepath, 
        aiProcess_FlipWindingOrder |
		aiProcess_Triangulate |
        aiProcess_ForceGenNormals |
		aiProcess_FlipUVs | 
		aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		CORE_LOG_ERROR("[MeshLoader] Assimp error loading {0}: {1}", filepath, importer.GetErrorString());
		return nullptr;
	}

	if (scene->mNumMeshes == 0) {
		CORE_LOG_ERROR("[MeshLoader] No meshes found in file: {0}", filepath);
		return nullptr;
	}

	aiMesh* mesh = scene->mMeshes[0];

	std::vector<Vertex> vertices;
	vertices.reserve(mesh->mNumVertices);

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;

        // Posici�n
        vertex.pos.x = mesh->mVertices[i].x;
        vertex.pos.y = mesh->mVertices[i].y;
        vertex.pos.z = mesh->mVertices[i].z;

        // Normales
        if (mesh->HasNormals()) {
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }
        else {
            vertex.normal = { 0.0f, 0.0f, 0.0f }; // O un valor por defecto
        }

        // Coordenadas de Textura (UVs)
        // Assimp permite hasta 8 sets de coordenadas de textura por v�rtice.
        // Normalmente, solo nos interesa el primero (�ndice 0).
        if (mesh->mTextureCoords[0]) {
            vertex.texCoord.x = mesh->mTextureCoords[0][i].x;
            vertex.texCoord.y = mesh->mTextureCoords[0][i].y;
        }
        else {
            vertex.texCoord = { 0.0f, 0.0f };
        }

        // Aqu� tambi�n podr�as extraer tangentes, bitangentes, colores de v�rtice, etc. si tu struct Vertex los soporta.
        // if (mesh->HasTangentsAndBitangents()) {
        //     vertex.tangent.x = mesh->mTangents[i].x; ...
        // }

        vertices.push_back(vertex);
    }
    // 5. Extraer los datos de los �ndices
    std::vector<unsigned int> indices;
    // Cada cara ('face') en Assimp es un primitivo (en nuestro caso, un tri�ngulo gracias a aiProcess_Triangulate).
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        // Recorremos los �ndices de la cara y los a�adimos a nuestra lista de �ndices.
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }
    if (vertices.empty() || indices.empty()) {
        CORE_LOG_WARN("[MeshLoader] Model loading resulted in an empty mesh for file: {0}", filepath);
        return nullptr; // Devuelve nullptr si no hay datos que procesar
    }

    CORE_LOG_INFO("[MeshLoader] Successfully loaded model data: {0} ({1} vertices, {2} indices)", filepath, vertices.size(), indices.size());

    return Mesh::CreateMeshFromData(
        vertices.data(),
        vertices.size() * sizeof(Vertex), // CreateMeshFromData espera el tama�o total en bytes
        indices.data(),
        indices.size());
}