#pragma once
#include "transform.hpp"
#include "material.hpp"
#include <stdio.h>

struct Vertex {
    glm::vec3 pos = glm::vec3(0.0f);
    glm::vec3 norm = glm::vec3(0.0f);
    glm::vec2 st = glm::vec2(0.0f); // uv-coord
    glm::vec4 col = glm::vec4(0.0f);
};

struct Mesh {
    Mesh() {
        glCreateVertexArrays(1, &vao); // vertex array object
        glCreateBuffers(1, &vbo); // vertex buffer object
        glCreateBuffers(1, &ebo); // element buffer object (or: index buffer object)
    }
    Mesh(aiMesh* pMesh) : Mesh() {
        load_mesh(pMesh);
    }
    ~Mesh() {
        std::vector<GLuint> buffers = { vao, vbo, ebo };
        glDeleteBuffers(buffers.size(), buffers.data());
    }

    void load_sphere(float nSectors, float nStacks, bool bInvertNormals = false) {
        // https://www.songho.ca/opengl/gl_sphere.html
        float pi = 3.14159265358979323846f;
        float radius = 0.5f;
        // precalc expensive operations
        float lengthInv = 1.0f / radius;
        float sectorStep = 2.0f * pi / static_cast<float>(nSectors);
        float stackStep = pi / static_cast<float>(nStacks);

        // preallocate some space for vertices
        vertices.reserve((nSectors + 1) * (nStacks + 1));

        // create vertices
        for (int i = 0; i <= nStacks; i++) {
            float stackAngle = pi / 2.0f - static_cast<float>(i) * stackStep;
            float xy = radius * std::cos(stackAngle);
            float z = radius * std::sin(stackAngle);

            for (int k = 0; k <= nSectors; k++) {
                Vertex& vertex = vertices.emplace_back();

                // calculate position
                float sectorAngle = static_cast<float>(k) * sectorStep;
                vertex.pos.x = xy * std::cos(sectorAngle);
                vertex.pos.y = xy * std::sin(sectorAngle);
                vertex.pos.z = z;

                // calculate normal
                if (bInvertNormals) vertex.norm = vertex.pos * -lengthInv;
                else vertex.norm = vertex.pos * lengthInv;

                // calculate uv/st coordinates
                // vertex.st.s = static_cast<float>(k) / nSectors;
                // vertex.st.t = static_cast<float>(i) / nStacks;
                vertex.col = glm::vec4(1, 1, 1, 1);
            }
        }

        // create indices
        // k1--k1+1
        // |  / |
        // | /  |
        // k2--k2+1
        for (GLuint i = 0; i < (GLuint)nStacks; i++) {
            GLuint k1 = i * (nSectors + 1); // beginning of current stack
            GLuint k2 = k1 + nSectors + 1;  // beginning of next stack

            for (GLuint j = 0; j < (GLuint)nSectors; j++, k1++, k2++) {
                // 2 triangles per sector excluding first and last stacks
                if (i != 0) {
                    indices.insert(indices.end(), {
                        k1, k2, k1 + 1
                    });
                }
                if (i != nStacks - 1) {
                    indices.insert(indices.end(), {
                        k1 + 1, k2, k2 + 1
                    });
                }
            }
        }

        describe_layout();
    }
    void load_cube(bool bInvertNormals = false) {
        float n = -0.5f; // for readability
        float p = 0.5f; // for readability
        vertices = {
            // pos, norm, uv/st, col
            {{n, n, p}, {0, 0, +1}, {0, 0}, {1, 1, 1, 1}}, // front
            {{p, n, p}, {0, 0, +1}, {0, 0}, {1, 1, 1, 1}},
            {{n, p, p}, {0, 0, +1}, {0, 0}, {1, 1, 1, 1}},
            {{p, p, p}, {0, 0, +1}, {0, 0}, {1, 1, 1, 1}},
            {{n, n, n}, {0, 0, -1}, {0, 0}, {1, 1, 1, 1}}, // back
            {{p, n, n}, {0, 0, -1}, {0, 0}, {1, 1, 1, 1}},
            {{n, p, n}, {0, 0, -1}, {0, 0}, {1, 1, 1, 1}},
            {{p, p, n}, {0, 0, -1}, {0, 0}, {1, 1, 1, 1}},
            {{n, n, n}, {-1, 0, 0}, {0, 0}, {1, 1, 1, 1}}, // left
            {{n, n, p}, {-1, 0, 0}, {0, 0}, {1, 1, 1, 1}},
            {{n, p, n}, {-1, 0, 0}, {0, 0}, {1, 1, 1, 1}},
            {{n, p, p}, {-1, 0, 0}, {0, 0}, {1, 1, 1, 1}},
            {{p, n, n}, {+1, 0, 0}, {0, 0}, {1, 1, 1, 1}}, // right
            {{p, n, p}, {+1, 0, 0}, {0, 0}, {1, 1, 1, 1}},
            {{p, p, n}, {+1, 0, 0}, {0, 0}, {1, 1, 1, 1}},
            {{p, p, p}, {+1, 0, 0}, {0, 0}, {1, 1, 1, 1}},
            {{n, p, n}, {0, +1, 0}, {0, 0}, {1, 1, 1, 1}}, // top
            {{n, p, p}, {0, +1, 0}, {0, 0}, {1, 1, 1, 1}},
            {{p, p, n}, {0, +1, 0}, {0, 0}, {1, 1, 1, 1}},
            {{p, p, p}, {0, +1, 0}, {0, 0}, {1, 1, 1, 1}},
            {{n, n, n}, {0, -1, 0}, {0, 0}, {1, 1, 1, 1}}, // bottom
            {{n, n, p}, {0, -1, 0}, {0, 0}, {1, 1, 1, 1}},
            {{p, n, n}, {0, -1, 0}, {0, 0}, {1, 1, 1, 1}},
            {{p, n, p}, {0, -1, 0}, {0, 0}, {1, 1, 1, 1}},
        };
        indices = {
            0, 1, 3, 3, 2, 0, // front
            5, 4, 7, 7, 4, 6, // back
            8, 9, 11, 11, 10, 8, // left
            13, 12, 15, 15, 12, 14, // right
            16, 17, 19, 19, 18, 16, // top
            23, 21, 20, 23, 20, 22, // bottom
        };

        if (bInvertNormals) invert_normals();
        describe_layout();
    }
    void load_mesh(aiMesh* pMesh) {
        // handle all vertices for this mesh
        vertices.reserve(pMesh->mNumVertices);
        for (int i = 0; i < pMesh->mNumVertices; i++) {
            Vertex vertex;

            // extract positions
            vertex.pos.x = pMesh->mVertices[i].x;
            vertex.pos.y = pMesh->mVertices[i].y;
            vertex.pos.z = pMesh->mVertices[i].z;
            // extract normals
            vertex.norm.x = pMesh->mNormals[i].x;
            vertex.norm.y = pMesh->mNormals[i].y;
            vertex.norm.z = pMesh->mNormals[i].z;
            // extract uv/st coords
            if (pMesh->HasTextureCoords(0)) {
                vertex.st.s = pMesh->mTextureCoords[0][i].x;
                vertex.st.t = pMesh->mTextureCoords[0][i].y;
            }
            // extract vertex colors (if present)
            if (pMesh->HasVertexColors(0)) {
                vertex.col.r = pMesh->mColors[0][i].r;
                vertex.col.g = pMesh->mColors[0][i].g;
                vertex.col.b = pMesh->mColors[0][i].b;
                vertex.col.a = pMesh->mColors[0][i].a;
            }

            vertices.push_back(vertex);
        }

        // handle all indices, each face is 3 indices
        indices.reserve(pMesh->mNumFaces * 3);
        for (int i = 0; i < pMesh->mNumFaces; i++) {
            aiFace face = pMesh->mFaces[i];
            for (int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }

        // simply assigned the material index for later
        materialIndex = pMesh->mMaterialIndex;
        describe_layout();
    }

    void draw() {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
    }

private:
    void describe_layout() {
        // describe vertex buffer
        GLsizeiptr nBytes = vertices.size() * sizeof(Vertex);
        glNamedBufferStorage(vbo, nBytes, vertices.data(), BufferStorageMask::GL_NONE_BIT);
        // describe index buffer
        nBytes = indices.size() * sizeof(GLuint);
        glNamedBufferStorage(ebo, nBytes, indices.data(), BufferStorageMask::GL_NONE_BIT);

        // describe buffer contents
        glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(Vertex));
        glVertexArrayElementBuffer(vao, ebo);
        // specify vertex format
        GLuint binding = 0;
        GLuint i;
        i = 0; // position
        glVertexArrayAttribFormat(vao, i, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(vao, i, binding);
        glEnableVertexArrayAttrib(vao, i);
        i = 1; // normal
        glVertexArrayAttribFormat(vao, i, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat));
        glVertexArrayAttribBinding(vao, i, binding);
        glEnableVertexArrayAttrib(vao, i);
        i = 2; // uv coordinate
        glVertexArrayAttribFormat(vao, i, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat));
        glVertexArrayAttribBinding(vao, i, binding);
        glEnableVertexArrayAttrib(vao, i);
        i = 3; // vertex color
        glVertexArrayAttribFormat(vao, i, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat));
        glVertexArrayAttribBinding(vao, i, binding);
        glEnableVertexArrayAttrib(vao, i);
    }
    void invert_normals() {
        for (Vertex& vertex : vertices) {
            vertex.norm *= -1.0f;
        }
    }

public:
    unsigned int materialIndex;

private:
    GLuint vao; // vertex array object
    GLuint vbo; // vertex buffer object
    GLuint ebo; // element buffer object
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
};