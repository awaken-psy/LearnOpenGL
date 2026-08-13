/**
 * Mesh 类 — 封装一个网格的渲染数据(VAO/VBO/EBO)和绘制逻辑
 *
 * 一个 Mesh 代表模型中的一个"零件"(比如背包的某个扣件、某块布料)。
 * Model 类加载模型时,把每个 aiMesh 转换成一个 Mesh 对象。
 *
 * 核心数据:
 *   - vertices:  顶点数组(位置/法线/纹理坐标/切线/副切线/骨骼)
 *   - indices:   索引数组(每 3 个索引组成一个三角形)
 *   - textures:  该网格使用的纹理列表(漫反射/镜面反射/法线/高度贴图)
 *
 * 渲染流程:
 *   1. Draw() 遍历 textures,按命名约定绑定到纹理单元
 *   2. 绑定 VAO
 *   3. glDrawElements 按索引绘制三角形
 *
 * ⭐ 纹理自动绑定机制:
 *   shader 中的 sampler2D 必须命名为 texture_<type><N>(如 texture_diffuse1)
 *   Draw() 会自动:计算编号 → glUniform1i 设置纹理单元 → glBindTexture 绑定纹理
 *   所以只要 shader 命名正确,cpp 端不需要手动绑定纹理。
 */

#ifndef MESH_H
#define MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <learnopengl/shader.h>

#include <string>
#include <vector>
using namespace std;

// 每个顶点最多受 4 根骨骼影响(骨骼动画用,本课暂不涉及)
#define MAX_BONE_INFLUENCE 4

/**
 * Vertex 结构体 — 描述一个顶点的所有属性
 *
 * ⭐ 利用 C++ 结构体的内存连续性:这些字段在内存中是顺序排列的,
 *   所以可以直接把 Vertex 数组传给 glBufferData,无需手动交错排列。
 *   offsetof(Vertex, 字段名) 用于获取字段的偏移量,配合 glVertexAttribPointer。
 */
struct Vertex {
    // 位置(世界空间坐标)
    glm::vec3 Position;
    // 法线(垂直于表面,光照计算用)
    glm::vec3 Normal;
    // 纹理坐标(UV)
    glm::vec2 TexCoords;
    // 切线(法线贴图的 TBN 矩阵用,定义纹理空间的 X 轴方向)
    glm::vec3 Tangent;
    // 副切线(TBN 矩阵用,定义纹理空间的 Y 轴方向)
    glm::vec3 Bitangent;
	// 骨骼索引(影响该顶点的骨骼编号,骨骼动画用)
	int m_BoneIDs[MAX_BONE_INFLUENCE];
	// 骨骼权重(每根骨骼的影响程度,4 个值之和应为 1.0)
	float m_Weights[MAX_BONE_INFLUENCE];
};

/**
 * Texture 结构体 — 描述一张已加载的纹理
 */
struct Texture {
    unsigned int id;   // OpenGL 纹理对象 ID(glGenTextures 生成)
    string type;       // 纹理类型:"texture_diffuse" / "texture_specular" / "texture_normal" / "texture_height"
    string path;       // 纹理文件路径(用于去重,避免重复加载)
};

class Mesh {
public:
    // ---- 网格数据 ----
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<Texture>      textures;
    unsigned int VAO;    // 公开,外部可能需要直接操作(如实例化时追加 VBO)

    // 构造函数:接收数据后立即调用 setupMesh 创建 GPU 缓冲区
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;

        // 数据齐了,创建 VAO/VBO/EBO 并设置顶点属性指针
        setupMesh();
    }

    /**
     * Draw — 绘制这个网格
     *
     * 两个步骤:
     *   1. 绑定纹理:按类型自动分配纹理单元(diffuse→0, specular→1, ...)
     *   2. 绘制:glDrawElements 按索引画三角形
     *
     * 纹理编号规则(与 shader 的 sampler 名对应):
     *   texture_diffuse1 → 纹理单元 0
     *   texture_diffuse2 → 纹理单元 1
     *   texture_specular1 → 纹理单元 2(接着 diffuse 往后排)
     *   ...
     */
    void Draw(Shader &shader) 
    {
        // ---- 绑定纹理 ----
        unsigned int diffuseNr  = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr   = 1;
        unsigned int heightNr   = 1;
        for(unsigned int i = 0; i < textures.size(); i++)
        {
            // 激活第 i 号纹理单元(GL_TEXTURE0 + i)
            glActiveTexture(GL_TEXTURE0 + i);

            // 根据纹理类型计算编号(取 N 值,如 texture_diffuse1 中的 "1")
            string number;
            string name = textures[i].type;
            if(name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if(name == "texture_specular")
                number = std::to_string(specularNr++);
            else if(name == "texture_normal")
                number = std::to_string(normalNr++);
             else if(name == "texture_height")
                number = std::to_string(heightNr++);

            // 设置 shader 中的 sampler uniform 指向第 i 号纹理单元
            // shader 里的变量名形如 "texture_diffuse1"
            glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
            // 把纹理对象绑定到当前纹理单元
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }
        
        // ---- 绘制网格 ----
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // 恢复默认纹理单元(良好的习惯,避免状态泄漏)
        glActiveTexture(GL_TEXTURE0);
    }

private:
    // 渲染数据(VBO 和 EBO 不需要外部访问)
    unsigned int VBO, EBO;

    /**
     * setupMesh — 创建 VAO/VBO/EBO 并设置顶点属性指针
     *
     * ⭐ 利用结构体内存布局技巧:
     *   Vertex 结构体的字段在内存中是连续排列的,所以可以直接把 &vertices[0] 传给 glBufferData。
     *   每个属性的偏移量用 offsetof(Vertex, 字段名) 获取,无需手动计算字节数。
     *
     * 顶点属性布局(7 个属性):
     *   location 0: 位置     (vec3, 偏移 0)
     *   location 1: 法线     (vec3, 偏移 offsetof Normal)
     *   location 2: 纹理坐标 (vec2, 偏移 offsetof TexCoords)
     *   location 3: 切线     (vec3, 偏移 offsetof Tangent)
     *   location 4: 副切线   (vec3, 偏移 offsetof Bitangent)
     *   location 5: 骨骼ID   (ivec4, 偏移 offsetof m_BoneIDs, 用 glVertexAttribIPointer)
     *   location 6: 骨骼权重 (vec4, 偏移 offsetof m_Weights)
     */
    void setupMesh()
    {
        // 创建 VAO(记录顶点属性配置)、VBO(存顶点数据)、EBO(存索引数据)
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // ---- VBO:上传顶点数据 ----
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // ⭐ sizeof(Vertex) 包含所有字段,vertices 在内存中连续,直接整体上传
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);  

        // ---- EBO:上传索引数据 ----
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // ---- 设置顶点属性指针 ----
        // stride = sizeof(Vertex):每顶点的步长是整个 Vertex 结构体的大小
        // ⭐ 用 offsetof 宏自动计算字段偏移,避免手动数字节

        // 位置 (location = 0)
        glEnableVertexAttribArray(0);	
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // 法线 (location = 1)
        glEnableVertexAttribArray(1);	
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // 纹理坐标 (location = 2)
        glEnableVertexAttribArray(2);	
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // 切线 (location = 3)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // 副切线 (location = 4)
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
        // 骨骼ID (location = 5) — 整数属性,用 glVertexAttribIPointer(不是 I→F 转换)
		glEnableVertexAttribArray(5);
		glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
        // 骨骼权重 (location = 6)
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));

        glBindVertexArray(0);
    }
};
#endif
