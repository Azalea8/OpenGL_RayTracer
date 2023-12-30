// 物体表面材质定义
struct Material {
    glm::vec3 emissive = glm::vec3(0.0f, 0.0f, 0.0f);  // 作为光源时的发光颜色
    glm::vec3 baseColor = glm::vec3(0, 0, 0);
    float subsurface = 0.0;
    float metallic = 0.0; // 金属度
    float specular = 0.0;
    float specularTint = 0.0;
    float roughness = 0.0; //粗造度
    float anisotropic = 0.0;
    float sheen = 0.0;
    float sheenTint = 0.0;
    float clearcoat = 0.0;
    float clearcoatGloss = 0.0;
    float IOR = 1.0;
    float transmission = 0.0; // 暂时作为类型。0: DIFFUSE, 1: MICROFACET
};

// 三角形定义
struct Triangle {
    glm::vec3 p1, p2, p3;    // 顶点坐标
    glm::vec3 n1, n2, n3;    // 顶点法线
    Material material;  // 材质
};

struct BVHNode {
    int left, right;    // 左右子树索引
    int n, index;       // 叶子节点信息
    glm::vec3 AA, BB;        // 碰撞盒
};

struct Triangle_encoded {
    glm::vec3 p1, p2, p3;    // 顶点坐标
    glm::vec3 n1, n2, n3;    // 顶点法线
    glm::vec3 emissive;      // 自发光参数
    glm::vec3 baseColor;     // 颜色
    glm::vec3 param1;        // (subsurface, metallic, specular)
    glm::vec3 param2;        // (specularTint, roughness, anisotropic)
    glm::vec3 param3;        // (sheen, sheenTint, clearcoat)
    glm::vec3 param4;        // (clearcoatGloss, IOR, transmission)
};

struct BVHNode_encoded {
    glm::vec3 childs;        // (left, right, 保留)
    glm::vec3 leafInfo;      // (n, index, 保留)
    glm::vec3 AA;
    glm::vec3 BB;
};



