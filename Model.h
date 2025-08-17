
#pragma once
#ifndef MODEL_H
#define MODEL_H
#include <string>
#include <vector>
#include <glad/glad.h>
#include <glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Shader.h"
#include "Mesh.h"
#include "stb_image.h"
#define Print(x) std::cout<<x<<std::endl;

unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);
class Model
{
public:
    /*  函数   */
    Model(std::string const& path, bool gamma = false):gammaCorrection(gamma)
    {
        loadModel(path);
    }
    void Draw(Shader& shader) {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    };

    void  BindOffsetVBO(glm::mat4* InstanceOffset, unsigned int amount, unsigned int &buffer) {
        
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &InstanceOffset[0], GL_STATIC_DRAW);
        for (unsigned int i = 0; i < meshes.size(); i++) {
            unsigned int VAO = meshes[i].VAO;
            glBindVertexArray(VAO);

            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
            glEnableVertexAttribArray(5);
            glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
            glEnableVertexAttribArray(6);
            glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

            glVertexAttribDivisor(3, 1);
            glVertexAttribDivisor(4, 1);
            glVertexAttribDivisor(5, 1);
            glVertexAttribDivisor(6, 1);

            glBindVertexArray(0);
           
        }
    }
    void DrawElementsInstanced(Shader& shader,unsigned int amount) {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].DrawElementsInstanced(shader,amount);

    }
   
private:
    /*  模型数据  */
    std::vector<Mesh> meshes;
    std::string directory;
    std::vector<Texture> textures_loaded;//表示已加载纹理
    bool gammaCorrection;

    /*  函数   */
    void loadModel(std::string const &path) {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        //aiProcess_GenNormals：如果模型不包含法向量的话，就为每个顶点创建法线。
        //aiProcess_SplitLargeMeshes：将比较大的网格分割成更小的子网格，如果你的渲染有最大顶点数限制，只能渲染较小的网格，那么它会非常有用。
        //aiProcess_OptimizeMeshes：和上个选项相反，它会将多个小网格拼接为一个大的网格，减少绘制调用从而进行优化。
        if (!scene||!scene->mRootNode||scene->mFlags& AI_SCENE_FLAGS_INCOMPLETE) {
            std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() <<std::endl;
            return;
        }
        directory = path.substr(0,path.find_last_of('/'));//取字符串子串
        processNode(scene->mRootNode, scene);

    };
    void processNode(aiNode* node, const aiScene* scene)//先根的深度遍历
    {
        // 处理节点所有的网格（如果有的话）
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.emplace_back(processMesh(mesh, scene));
        }
        // 接下来对它的子节点重复这一过程
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    };
    Mesh processMesh(aiMesh* mesh, const aiScene* scene)//将assimp数据解析到Mesh类
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;
        //处理顶点
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
          
            Vertex vertex;
            vertex.Position = glm::vec3(
                mesh->mVertices[i].x, 
                mesh->mVertices[i].y, 
                mesh->mVertices[i].z);

            if (mesh->HasNormals())
            {
                vertex.Normal = glm::vec3(
                    mesh->mNormals[i].x,
                    mesh->mNormals[i].y,
                    mesh->mNormals[i].z);
            }
            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vertex.TexCoords = glm::vec2(
                    mesh->mTextureCoords[0][i].x, 
                    mesh->mTextureCoords[0][i].y);
                // tangent
                vertex.Tangent = glm::vec3(
                    mesh->mTangents[i].x, 
                    mesh->mTangents[i].y,
                    mesh->mTangents[i].z);
                // bitangent
                vertex.Bitangent = glm::vec3(
                    mesh->mBitangents[i].x, 
                    mesh->mBitangents[i].y, 
                    mesh->mBitangents[i].z);
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.emplace_back(vertex);


        }
        //处理索引
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.emplace_back(face.mIndices[j]);
        }
        //处理材质
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_height");//obj会加载法线贴图到这个类型,这个本来是高度贴图或深度贴图
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        std::vector<Texture> ambientMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_ambient");
        textures.insert(textures.end(), ambientMaps.begin(), ambientMaps.end());
        return  Mesh(vertices,indices,textures);

    };
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,const std::string& typeName) {
        std::vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                   
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if (!skip) {
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(),this->directory);
                texture.type=typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }
        return textures;
    };
};

unsigned int TextureFromFile(const char* Path,const std::string &directory, bool gamma) {
    std::string filename = std::string(Path);
    filename = directory + '/' + filename;
    unsigned int textureID;
    glGenTextures(1, &textureID);
    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
       
        GLenum format= GL_RED;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
     
      
    }
    else { Print("Fail to load texture") }
    stbi_image_free(data);
    return textureID;

}









#endif