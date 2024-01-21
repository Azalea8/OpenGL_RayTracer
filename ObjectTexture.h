#pragma once
#ifndef __ObjectTexture_h__
#define __ObjectTexture_h__


#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "Shader.h"

#include <vector>

class ObjectTexture {
public:
	GLuint trianglesTextureBuffer;
	GLuint nodesTextureBuffer;
	int nTriangles, nNodes;

	void setTex(Shader &shader) {

		shader.setInt("nTriangles", nTriangles);
		shader.setInt("nNodes", nNodes);

		glActiveTexture(GL_TEXTURE0 + 1);
		// and finally bind the texture
        glBindTexture(GL_TEXTURE_BUFFER, trianglesTextureBuffer);

        // 激活并绑定纹理
		glActiveTexture(GL_TEXTURE0 + 2);
		// and finally bind the texture
        glBindTexture(GL_TEXTURE_BUFFER, nodesTextureBuffer);

		shader.setInt("triangles", 1);
		shader.setInt("nodes", 2);
	}

    void getTexture_BVH(std::vector<BVHNode>& nodes, Shader& shader, int nNodes){
        std::vector<BVHNode_encoded> nodes_encoded(nNodes);
        for (int i = 0; i < nNodes; i++) {
            nodes_encoded[i].childs = glm::vec3(nodes[i].left, nodes[i].right, 0);
            nodes_encoded[i].leafInfo = glm::vec3(nodes[i].n, nodes[i].index, 0);
            nodes_encoded[i].AA = nodes[i].AA;
            nodes_encoded[i].BB = nodes[i].BB;
        }

        this -> nNodes = nNodes;

        shader.use();

        GLuint tbo_2;
        glGenBuffers(1, &tbo_2);
        glBindBuffer(GL_TEXTURE_BUFFER, tbo_2);

        glBufferData(GL_TEXTURE_BUFFER, nodes_encoded.size() * sizeof(BVHNode_encoded), &nodes_encoded[0], GL_STATIC_DRAW);

        glGenTextures(1, &nodesTextureBuffer);
        glBindTexture(GL_TEXTURE_BUFFER, nodesTextureBuffer);

        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo_2);

        // 删除数组
        nodes_encoded.clear();

        // 释放内存
        nodes_encoded.shrink_to_fit();
    }

    void getTexture_Triangle(std::vector<Triangle>& triangles, Shader& shader, int nTriangles) {
        std::vector<Triangle_encoded> triangles_encoded(nTriangles);
        for (int i = 0; i < nTriangles; i++) {
            Triangle& t = triangles[i];
            Material& m = t.material;
            // 顶点位置
            triangles_encoded[i].p1 = t.p1;
            triangles_encoded[i].p2 = t.p2;
            triangles_encoded[i].p3 = t.p3;
            // 顶点法线
            triangles_encoded[i].n1 = t.n1;
            triangles_encoded[i].n2 = t.n2;
            triangles_encoded[i].n3 = t.n3;
            // 材质
            triangles_encoded[i].emissive = m.emissive;
            triangles_encoded[i].baseColor = m.baseColor;
            triangles_encoded[i].param1 = glm::vec3(m.subsurface, m.metallic, m.specular);
            triangles_encoded[i].param2 = glm::vec3(m.specularTint, m.roughness, m.anisotropic);
            triangles_encoded[i].param3 = glm::vec3(m.sheen, m.sheenTint, m.clearcoat);
            triangles_encoded[i].param4 = glm::vec3(m.clearcoatGloss, m.IOR, m.transmission);
        }

        this -> nTriangles = nTriangles;

        shader.use();

        GLuint tbo_1;
        glGenBuffers(1, &tbo_1);
        glBindBuffer(GL_TEXTURE_BUFFER, tbo_1);

        glBufferData(GL_TEXTURE_BUFFER, triangles_encoded.size() * sizeof(Triangle_encoded), &triangles_encoded[0], GL_STATIC_DRAW);

        glGenTextures(1, &trianglesTextureBuffer);
        glBindTexture(GL_TEXTURE_BUFFER, trianglesTextureBuffer);

        glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo_1);

        // 删除数组
        triangles_encoded.clear();

        // 释放内存
        triangles_encoded.shrink_to_fit();
    }
};







#endif






