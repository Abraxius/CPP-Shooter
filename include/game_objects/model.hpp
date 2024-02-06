#pragma once
#include <string>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/material.h>
#include "utils.hpp"
#include "mesh.hpp"
#include "material.hpp"
#include "cmrc_io.hpp"

// https://github.com/jimmiebergmann/Sponza
struct Model {
    Model(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale, std::string path)
    : transform(pos, rot, scale) {
        Assimp::Importer importer;

        // flags that allow some automatic post processing of model
        unsigned int flags = 0; // https://assimp.sourceforge.net/lib_html/postprocess_8h.html
        flags |= aiProcess_Triangulate; // triangulate all faces if not already triangulated
        flags |= aiProcess_GenNormals; // generate normals if they dont exist
        flags |= aiProcess_FlipUVs; // OpenGL prefers flipped y axis
        flags |= aiProcess_PreTransformVertices; // simplifies model load
        
        // load model
        #ifdef EMBEDDED_MODELS
        importer.SetIOHandler(new CMRC_IOSystem()); // custom virtual IO system for embedded resources
        #else
        path = "../" + path; // adjust path when reading from disk
        #endif
        const aiScene* pScene = importer.ReadFile(path, flags);
        if (pScene == nullptr) {
            std::cerr << importer.GetErrorString() << '\n';
            return;
        }
        else std::cout << "Loaded model: " << path << std::endl;

        // we need to figure out the path the root of a model for formats such as .obj
        size_t sepIndex = path.find_last_of('/');
        modelRoot = path.substr(0, sepIndex + 1);

        // create meshes
        meshes.reserve(pScene->mNumMeshes);
        for (int i = 0; i < pScene->mNumMeshes; i++) {
            aiMesh* pMesh = pScene->mMeshes[i];
            meshes.emplace_back(pMesh);
        }

        // create textures (if embedded into model, such as .glb)
        textures.reserve(pScene->mNumTextures);
        for (int i = 0; i < pScene->mNumTextures; i++) {
            aiTexture* pTexture = pScene->mTextures[i];
            GLuint texture;
            
            // check if embedded texture is compressed
            if (pTexture->mHeight == 0) {
                texture = load_texture(pTexture);
            }
            else std::cerr << "uncompressed embedded images not handled yet\n";
        }

        // create materials
        // https://assimp.sourceforge.net/lib_html/materials.html
        materials.resize(pScene->mNumMaterials);
        for (int i = 0; i < pScene->mNumMaterials; i++) {
            aiMaterial* pMaterial = pScene->mMaterials[i];
            Material& material = materials[i];
            aiColor3D color;

            // Load basic material properties
            pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color);
            material.ambient = { color.r, color.g, color.b };
            pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            material.diffuse = { color.r, color.g, color.b };
            pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
            material.specular = { color.r, color.g, color.b };
            pMaterial->Get(AI_MATKEY_SHININESS, material.shininess);
            pMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, material.shininessStrength);

            // load diffuse texture
            if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE)) {
                material.diffuseTexture = get_texture(pMaterial, aiTextureType_DIFFUSE);
                material.diffuseBlend = 1.0f;
            }
        }

    }
    void draw() {
        transform.bind();
        for (int i = 0; i < meshes.size(); i++) {
            Material& material = materials[meshes[i].materialIndex];
            material.bind();
            meshes[i].draw();
        }
    }

private:
    GLuint get_texture(aiMaterial* pMaterial, aiTextureType texType) {
        // build path to texture resource
        aiString aiTexPath;
        pMaterial->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), aiTexPath);

        // check if we previously loaded this texture
        auto textureMapIter = textures.find(aiTexPath.C_Str());
        if (textureMapIter != textures.end()) return textureMapIter->second;

        // load texture from memory using stbi
        std::string texPath(modelRoot);
        texPath.append(aiTexPath.C_Str());
        int width, height, nChannels;
        #ifdef EMBEDDED_MODELS
        auto rawTex = load_model_resource(texPath);
        stbi_uc* pImage = stbi_load_from_memory(rawTex.first, rawTex.second, &width, &height, &nChannels, 4);
        #else
        stbi_uc* pImage = stbi_load(texPath.c_str(), &width, &height, &nChannels, 4);
        #endif
        if (pImage == nullptr) {
            std::cerr << "failed to load model texture" << std::endl;
            return 0;
        }

        // create texture
        GLuint texture = create_gl_tex(pImage, width, height);
        stbi_image_free(pImage);

        // add texture to our lookup map
        textures[aiTexPath.C_Str()] = texture;
        return texture;
    }
    GLuint load_texture(aiTexture* pTexture) {
        // uncompress using stbi
        int width, height, nChannels;
        const stbi_uc* pCompressedTexture = reinterpret_cast<stbi_uc*>(pTexture->pcData); // cast to a type that stbi understands
        stbi_uc* pImage = stbi_load_from_memory(pCompressedTexture, pTexture->mWidth, &width, &height, &nChannels, 4);
        if (pImage == nullptr) std::cerr << "failed to load embedded model texture" << std::endl;

        // create texture
        GLuint texture = create_gl_tex(pImage, width, height);
        stbi_image_free(pImage);

        // add texture to our lookup map
        textures[pTexture->mFilename.C_Str()] = texture;

        return texture;
    }
    GLuint create_gl_tex(void* pImage, int width, int height) {
        GLuint texture;
        glCreateTextures(GL_TEXTURE_2D, 1, &texture);
        glTextureStorage2D(texture, 1, GL_RGBA8, width, height);
        glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pImage);

        // set wrapping/magnification behavior
        glTextureParameteri(texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(texture, GL_TEXTURE_WRAP_T, GL_REPEAT);

        // mip-maps
        glGenerateTextureMipmap(texture);
        glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // anisotropic filtering
        GLfloat anisotropicFiltering;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &anisotropicFiltering);
        anisotropicFiltering = std::min(anisotropicFiltering, 8.0f);
        glTextureParameterf(texture, GL_TEXTURE_MAX_ANISOTROPY, anisotropicFiltering);
        return texture;
    }

    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    std::unordered_map<std::string, GLuint> textures;
    std::string modelRoot;
public:
    Transform transform;
};