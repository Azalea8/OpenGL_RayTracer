#include <algorithm>

// ������������������ -- �ȽϺ���
bool cmpx(const Triangle& t1, const Triangle& t2) {
    glm::vec3 center1 = (t1.p1 + t1.p2 + t1.p3) / glm::vec3(3, 3, 3);
    glm::vec3 center2 = (t2.p1 + t2.p2 + t2.p3) / glm::vec3(3, 3, 3);
    return center1.x < center2.x;
}
bool cmpy(const Triangle& t1, const Triangle& t2) {
    glm::vec3 center1 = (t1.p1 + t1.p2 + t1.p3) / glm::vec3(3, 3, 3);
    glm::vec3 center2 = (t2.p1 + t2.p2 + t2.p3) / glm::vec3(3, 3, 3);
    return center1.y < center2.y;
}
bool cmpz(const Triangle& t1, const Triangle& t2) {
    glm::vec3 center1 = (t1.p1 + t1.p2 + t1.p3) / glm::vec3(3, 3, 3);
    glm::vec3 center2 = (t2.p1 + t2.p2 + t2.p3) / glm::vec3(3, 3, 3);
    return center1.z < center2.z;
}

// ���������������
int buildBVH(std::vector<Triangle>& triangles, std::vector<BVHNode>& nodes, int l, int r, int num) {
    if (l > r) return 0;

    // ע��
    // �˴�����ͨ��ָ�룬���õȷ�ʽ������������ nodes[id] ������
    // ��Ϊ std::vector<> ����ʱ�´����������ڴ棬��ô��ַ�͸ı���
    // ��ָ�룬���þ�ָ��ԭ�����ڴ棬���Իᷢ������
    nodes.push_back(BVHNode());
    int id = nodes.size() - 1;   // ע�⣺ �ȱ�������

    nodes[id].left = nodes[id].right = nodes[id].n = nodes[id].index = 0;

    nodes[id].AA = glm::vec3(1145141919, 1145141919, 1145141919);
    nodes[id].BB = glm::vec3(-1145141919, -1145141919, -1145141919);

    // ���� AABB��Χ��
    for (int i = l; i <= r; i++) {
        // ��С�� AA
        float minx = std::min(triangles[i].p1.x, std::min(triangles[i].p2.x, triangles[i].p3.x));
        float miny = std::min(triangles[i].p1.y, std::min(triangles[i].p2.y, triangles[i].p3.y));
        float minz = std::min(triangles[i].p1.z, std::min(triangles[i].p2.z, triangles[i].p3.z));
        nodes[id].AA.x = std::min(nodes[id].AA.x, minx);
        nodes[id].AA.y = std::min(nodes[id].AA.y, miny);
        nodes[id].AA.z = std::min(nodes[id].AA.z, minz);

        // ���� BB
        float maxx = std::max(triangles[i].p1.x, std::max(triangles[i].p2.x, triangles[i].p3.x));
        float maxy = std::max(triangles[i].p1.y, std::max(triangles[i].p2.y, triangles[i].p3.y));
        float maxz = std::max(triangles[i].p1.z,std::max(triangles[i].p2.z, triangles[i].p3.z));
        nodes[id].BB.x = std::max(nodes[id].BB.x, maxx);
        nodes[id].BB.y = std::max(nodes[id].BB.y, maxy);
        nodes[id].BB.z = std::max(nodes[id].BB.z, maxz);
    }

    // ������ n �������� ����Ҷ�ӽڵ�
    if ((r - l + 1) <= num) {
        nodes[id].n = r - l + 1;
        nodes[id].index = l;
        return id;
    }

    // ����ݹ齨��
    float lenx = nodes[id].BB.x - nodes[id].AA.x;
    float leny = nodes[id].BB.y - nodes[id].AA.y;
    float lenz = nodes[id].BB.z - nodes[id].AA.z;

    // �� x ����
    if (lenx >= leny && lenx >= lenz)
        std::sort(triangles.begin() + l, triangles.begin() + r + 1, cmpx);
    // �� y ����
    else if (leny >= lenx && leny >= lenz)
        std::sort(triangles.begin() + l, triangles.begin() + r + 1, cmpy);
    // �� z ����
    else if (lenz >= lenx && lenz >= leny)
        std::sort(triangles.begin() + l, triangles.begin() + r + 1, cmpz);
    // �ݹ�
    int mid = (l + r) / 2;
    int left = buildBVH(triangles, nodes, l, mid, num);
    int right = buildBVH(triangles, nodes, mid + 1, r, num);

    nodes[id].left = left;
    nodes[id].right = right;

    return id;
}