/**
 * Model 类 — 基于 Assimp 的 3D 模型加载器
 *
 * 职责:读取 .obj/.fbx 等模型文件 → 递归遍历场景节点树 → 提取所有 Mesh → 加载纹理
 *
 * 核心流程:
 *   1. Assimp::Importer.ReadFile() 读取文件,返回 aiScene(场景对象)
 *   2. processNode() 递归遍历节点树(节点本身不含数据,只存索引)
 *   3. processMesh() 把 Assimp 的 aiMesh 转成我们的 Mesh 对象(顶点/索引/纹理)
 *   4. loadMaterialTextures() 加载材质引用的纹理文件,缓存避免重复加载
 *
 * Assimp 后处理标志(传入 ReadFile):
 *   - aiProcess_Triangulate:   把非三角形面(四边形等)全部转成三角形
 *   - aiProcess_GenSmoothNormals: 对没有法线的模型自动生成平滑法线
 *   - aiProcess_FlipUVs:       翻转纹理 Y 轴(适配 OpenGL 的左下角原点)
 *   - aiProcess_CalcTangentSpace: 自动计算切线/副切线(法线贴图需要)
 *
 * 纹理命名约定(与 Mesh::Draw 配合):
 *   shader 中的 sampler2D 必须命名为 texture_diffuseN / texture_specularN / texture_normalN / texture_heightN
 *   N 从 1 开始递增,Mesh::Draw 按此命名自动绑定到对应纹理单元
 */

#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <learnopengl/mesh.h>
#include <learnopengl/shader.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

// 前向声明:加载纹理文件的工具函数(定义在文件末尾)
unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);

class Model 
{
public:
    // ---- 模型数据 ----
    vector<Texture> textures_loaded;	// 【缓存】已加载过的纹理,避免同一纹理被重复加载多次(优化)
    vector<Mesh>    meshes;            // 模型包含的所有网格
    string directory;                  // 模型文件所在目录(纹理路径是相对的,需要拼接目录)
    bool gammaCorrection;              // 是否启用伽马校正

    // 构造函数:传入模型文件路径,内部调用 loadModel 完成加载
    Model(string const &path, bool gamma = false) : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    // ---- 渲染:遍历所有 Mesh,逐个调用 Mesh::Draw ----
    void Draw(Shader &shader)
    {
        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }
    
private:
    /**
     * loadModel — 读取模型文件,启动节点递归处理
     * Assimp::Importer 在栈上创建,ReadFile 返回的 aiScene 由 importer 持有,
     * importer 析构时会自动释放场景数据,所以不能把 scene 存到成员变量里。
     */
    void loadModel(string const &path)
    {
        // 读取文件,应用后处理标志
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, 
            aiProcess_Triangulate |      // 面转三角形
            aiProcess_GenSmoothNormals | // 自动生成法线
            aiProcess_FlipUVs |          // 翻转纹理 Y 轴
            aiProcess_CalcTangentSpace); // 计算切线/副切线(法线贴图用)

        // 错误检查:文件读取失败 / 场景不完整 / 没有根节点
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // 提取模型文件所在目录(去掉文件名),用于后续拼接纹理路径
        directory = path.substr(0, path.find_last_of('/'));

        // 从根节点开始递归处理
        processNode(scene->mRootNode, scene);
    }

    /**
     * processNode — 递归遍历节点树
     *
     * Assimp 的场景结构:
     *   aiScene
     *     └─ mRootNode (aiNode)
     *          ├─ mMeshes[] (索引,指向 scene->mMeshes 里的实际数据)
     *          └─ mChildren[] (子节点,递归处理)
     *
     * 节点本身不存网格数据,只存索引 — 这是为了支持实例化(同一网格可被多个节点引用)。
     */
    void processNode(aiNode *node, const aiScene *scene)
    {
        // ---- 处理当前节点包含的网格 ----
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // node->mMeshes[i] 是索引,用它在 scene->mMeshes 里取实际的 aiMesh
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // ---- 递归处理子节点 ----
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    /**
     * processMesh — 把 Assimp 的 aiMesh 转换成我们的 Mesh 对象
     *
     * 提取三类数据:
     *   1. 顶点:位置、法线、纹理坐标、切线、副切线
     *   2. 索引:面(三角形)的顶点索引,用于 EBO
     *   3. 纹理:从材质中加载漫反射/镜面反射/法线/高度贴图
     */
    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // ---- 1. 提取顶点数据 ----
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            // Assimp 用自己的 aiVector3D 类,不能直接赋给 glm::vec3,需要逐分量拷贝
            glm::vec3 vector; // 临时变量,做类型转换用

            // 位置
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            // 法线
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }

            // 纹理坐标
            // ⭐ Assimp 允许每个顶点最多 8 组纹理坐标,这里只取第 0 组(大多数模型只有一组)
            if(mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;

                // 切线和副切线(由 aiProcess_CalcTangentSpace 自动生成,法线贴图需要)
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;

                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        // ---- 2. 提取索引数据 ----
        // 遍历所有面(face),每个面是一个三角形(aiProcess_Triangulate 保证)
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // 把面的所有顶点索引加入 indices 数组
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);        
        }

        // ---- 3. 提取纹理(材质) ----
        // 每个网格关联一个材质(scene->mMaterials),材质里引用了多张纹理
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    

        // ⭐ 纹理命名约定:sampler 名必须为 texture_<type>N(N 从 1 开始)
        //   diffuse:  texture_diffuseN
        //   specular: texture_specularN
        //   normal:   texture_normalN
        //   height:   texture_heightN
        // Mesh::Draw 按此命名自动设置 uniform + 绑定纹理单元

        // 1. 漫反射贴图
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. 镜面反射贴图
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. 法线贴图(Assimp 中 aiTextureType_HEIGHT 对应法线贴图)
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. 高度贴图
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        
        // 返回组装好的 Mesh(构造函数内部会调用 setupMesh 创建 VAO/VBO/EBO)
        return Mesh(vertices, indices, textures);
    }

    /**
     * loadMaterialTextures — 从材质中加载指定类型的纹理
     *
     * ⭐ 去重优化:遍历 textures_loaded 缓存,如果纹理路径已存在则跳过加载,
     *   复用已有的 Texture 对象。这对多网格共用同一纹理的模型特别重要
     *   (比如背包的金属扣件和拉链可能共用同一张镜面反射贴图)。
     */
    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            // 检查是否已加载过
            bool skip = false;
            for(unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    // 已加载过,直接复用
                    textures.push_back(textures_loaded[j]);
                    skip = true;
                    break;
                }
            }
            if(!skip)
            {
                // 未加载过,加载纹理文件
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // 加入缓存
            }
        }
        return textures;
    }
};


/**
 * TextureFromFile — 从文件加载纹理的工具函数
 *
 * 封装了 stbi_load + glTexImage2D + glGenerateMipmap + 参数设置的完整流程。
 * 根据图片实际通道数选择对应的 GL 格式:
 *   1 通道 → GL_RED (灰度图)
 *   3 通道 → GL_RGB
 *   4 通道 → GL_RGBA (含透明度)
 */
unsigned int TextureFromFile(const char *path, const string &directory, bool gamma)
{
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // 纹理参数:平铺方式 = 重复,缩小过滤 = 线性+多级渐远,放大过滤 = 线性
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
#endif
