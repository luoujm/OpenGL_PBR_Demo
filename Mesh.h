#pragma once
#ifndef MESH_H
#define MESH_H
#include <string>
#include <vector>
#include <glad/glad.h>
#include <glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include "Shader.h"
#define Print(x) std::cout<<x<<std::endl;
#define MAX_BONE_INFLUENCE 4

//c++结构体变量是连续，按序布局
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
    unsigned int id;//openGL生成的texture对象编号
    std::string type;
    std::string path;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    unsigned int VAO;
   
    Mesh( std::vector<Vertex> &vertices,std::vector<unsigned int> &indices,std::vector<Texture> &textures) {
        this->vertices.swap(vertices);
        this->indices.swap(indices);
        this->textures.swap(textures);

        setupMesh();
    };
    Mesh(const Mesh& other) {
        this->VAO = other.VAO;
        this->VBO = other.VBO;
        this->EBO = other.EBO;
        this->vertices=other.vertices;
        this->indices=other.indices;
        this->textures=other.textures;
    }
    Mesh(Mesh&& other) noexcept{
        this->vertices.swap(other.vertices);
        this->indices.swap(other.indices);
        this->textures.swap(other.textures);
        this->VAO = other.VAO;
        this->VBO = other.VBO;
        this->EBO = other.EBO;
    }
    void Draw(Shader &shader) {
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        unsigned int ambientNr = 1;
        for (unsigned int i = 0; i < textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);//GL_TEXTURE0+1=GL_TEXTURE1
            if (textures[i].type == "texture_diffuse") { shader.setInt(("material.texture_diffuse" + std::to_string(diffuseNr++)), i); }
            else if (textures[i].type == "texture_specular") { shader.setInt(("material.texture_specular" + std::to_string(specularNr++)), i); }
            else if (textures[i].type == "texture_normal") { shader.setInt(("material.texture_normal" + std::to_string(normalNr++)), i); }
            else if (textures[i].type == "texture_height") { shader.setInt(("material.texture_height" + std::to_string(heightNr++)), i); }
            else if (textures[i].type == "texture_ambient") { shader.setInt(("material.texture_ambient" + std::to_string(ambientNr++)), i); }
            glBindTexture(GL_TEXTURE_2D, textures[i].id);//按照数组顺序将漫反射贴图加载给依次顺序的采样器
        }

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);//形参列表：图元类型，绘制顶点个数，索引数据类型，偏移量
        glBindVertexArray(0);
    };
    void DrawElementsInstanced(Shader& shader,unsigned int amount) {
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        unsigned int ambientNr = 1;
        for (unsigned int i = 0; i < textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);//GL_TEXTURE0+1=GL_TEXTURE1
            if (textures[i].type == "texture_diffuse") { shader.setInt(("material.texture_diffuse" + std::to_string(diffuseNr++)), i); }
            else if (textures[i].type == "texture_specular") { shader.setInt(("material.texture_specular" + std::to_string(specularNr++)), i); }
            else if (textures[i].type == "texture_normal") { shader.setInt(("material.texture_normal" + std::to_string(normalNr++)), i); }
            else if (textures[i].type == "texture_height") { shader.setInt(("material.texture_height" + std::to_string(heightNr++)), i); }
            else if (textures[i].type == "texture_ambient") { shader.setInt(("material.texture_ambient" + std::to_string(ambientNr++)), i); }
            glBindTexture(GL_TEXTURE_2D, textures[i].id);//按照数组顺序将漫反射贴图加载给依次顺序的采样器
        }

        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, amount);
        glBindVertexArray(0);
    };

    Mesh& operator=(const Mesh& other) {
        this->VAO = other.VAO;
        this->VBO = other.VBO;
        this->EBO = other.EBO;
        this->vertices = other.vertices;
        this->indices = other.indices;
        this->textures = other.textures;
        return *this;
    }
    Mesh& operator=( Mesh&& other)noexcept {
        if (this != &other) {
            this->vertices.swap(other.vertices);
            this->indices.swap(other.indices);
            this->textures.swap(other.textures);
            this->VAO = other.VAO;
            this->VBO = other.VBO;
            this->EBO = other.EBO;
        }
        return *this;
    }
    
private:
    unsigned int  VBO, EBO;
    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        // 顶点位置
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, sizeof(Vertex::Position)/ sizeof(float), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // 顶点法线
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, sizeof(Vertex::Normal) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));//offsetof返回变量起始位置距离结构体起始位置的字节偏移量,转化成空指针方便地址运算
        // 顶点纹理坐标
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, sizeof(Vertex::TexCoords) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, sizeof(Vertex::Tangent) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, sizeof(Vertex::Bitangent) / sizeof(float), GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
        // ids
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, MAX_BONE_INFLUENCE, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
        // weights
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, MAX_BONE_INFLUENCE, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));

        glBindVertexArray(0);
     

    }

};







#endif