// ���������ʶ���
struct Material {
    glm::vec3 emissive = glm::vec3(0.0f, 0.0f, 0.0f);  // ��Ϊ��Դʱ�ķ�����ɫ
    glm::vec3 baseColor = glm::vec3(0, 0, 0);
    float subsurface = 0.0;
    float metallic = 0.0; // ������
    float specular = 0.0;
    float specularTint = 0.0;
    float roughness = 0.0; //�����
    float anisotropic = 0.0;
    float sheen = 0.0;
    float sheenTint = 0.0;
    float clearcoat = 0.0;
    float clearcoatGloss = 0.0;
    float IOR = 1.0;
    float transmission = 0.0; // ��ʱ��Ϊ���͡�0: DIFFUSE, 1: MICROFACET
};

// �����ζ���
struct Triangle {
    glm::vec3 p1, p2, p3;    // ��������
    glm::vec3 n1, n2, n3;    // ���㷨��
    Material material;  // ����
};

struct BVHNode {
    int left, right;    // ������������
    int n, index;       // Ҷ�ӽڵ���Ϣ
    glm::vec3 AA, BB;        // ��ײ��
};

struct Triangle_encoded {
    glm::vec3 p1, p2, p3;    // ��������
    glm::vec3 n1, n2, n3;    // ���㷨��
    glm::vec3 emissive;      // �Է������
    glm::vec3 baseColor;     // ��ɫ
    glm::vec3 param1;        // (subsurface, metallic, specular)
    glm::vec3 param2;        // (specularTint, roughness, anisotropic)
    glm::vec3 param3;        // (sheen, sheenTint, clearcoat)
    glm::vec3 param4;        // (clearcoatGloss, IOR, transmission)
};

struct BVHNode_encoded {
    glm::vec3 childs;        // (left, right, ����)
    glm::vec3 leafInfo;      // (n, index, ����)
    glm::vec3 AA;
    glm::vec3 BB;
};



