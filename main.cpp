#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.h"
#include "Structure.hpp"
#include "Model.hpp"
#include "BVH.hpp"
#include "Camera.h"
#include "RT_Screen.h"
#include "Tool.h"
#include "ObjectTexture.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

unsigned int SCR_WIDTH = 1200;
unsigned int SCR_HEIGHT = 800;

timeRecord tRecord;

Camera cam(SCR_WIDTH, SCR_HEIGHT);

RenderBuffer screenBuffer;

ObjectTexture ObjTex;

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "RayTracing", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	CPURandomInit();

	Shader RayTracerShader("../RayTracerVertexShader.glsl", "../RayTracerFragmentShader.glsl");
	Shader ScreenShader("../ScreenVertexShader.glsl", "../ScreenFragmentShader.glsl");

	RT_Screen screen;
	screen.InitScreenBind();

	screenBuffer.Init(SCR_WIDTH, SCR_HEIGHT);

    std::vector<Triangle> triangles;
    Material m;
    m.baseColor = glm::vec3(1, 1, 1);

    Material light;
    light.transmission = -1.0f;
    light.emissive = 8.0f * glm::vec3(0.747f+0.058f, 0.747f+0.258f, 0.747f) + 15.6f * glm::vec3 (0.740f+0.287f,0.740f+0.160f,0.740f) + 18.4f * glm::vec3(0.737f+0.642f,0.737f+0.159f,0.737f);

    Material red;
    red.transmission = 0.0f;
    red.baseColor = glm::vec3(0.63f, 0.065f, 0.05f);

    Material green;
    red.transmission = 0.0f;
    green.baseColor = glm::vec3(0.14f, 0.45f, 0.091f);

    Material white;
    white.transmission = 0.0f;
    white.baseColor = glm::vec3(0.725f, 0.71f, 0.68f);

    Material micro_Al;
    micro_Al.transmission = 1.0f;
    micro_Al.baseColor = glm::vec3(0.913f, 0.921f, 0.925f);
    micro_Al.roughness = 0.01;
    micro_Al.metallic = 1.0f;

    Material micro_Au;
    micro_Au.transmission = 1.0f;
    micro_Au.baseColor = glm::vec3(1.000f, 0.766f, 0.336f);
    micro_Au.roughness = 0.5f;
    micro_Au.metallic = 0.8f;

    readObj("../models/cornellbox/tallbox.obj", triangles, white, getTransformMatrix(glm::vec3(0, 0, 0), glm::vec3(0.0, 0.0, -0.0), glm::vec3(1,1, 1)) , false);
    readObj("../models/cornellbox/shortbox.obj", triangles, white, getTransformMatrix(glm::vec3(0, 0, 0), glm::vec3(0.0, 0.0, -0.0), glm::vec3(1,1, 1)) , false);
    readObj("../models/cornellbox/floor.obj", triangles, white, getTransformMatrix(glm::vec3(0, 0, 0), glm::vec3(0.0, 0.0, -0.0), glm::vec3(1,1, 1)) , false);
    readObj("../models/cornellbox/right.obj", triangles, green, getTransformMatrix(glm::vec3(0, 0, 0), glm::vec3(0.0, 0.0, -0.0), glm::vec3(1,1, 1)) , false);
    readObj("../models/cornellbox/left.obj", triangles, red, getTransformMatrix(glm::vec3(0, 0, 0), glm::vec3(0.0, 0.0, -0.0), glm::vec3(1,1, 1)) , false);
    readObj("../models/cornellbox/light.obj", triangles, light, getTransformMatrix(glm::vec3(0, 0, 0), glm::vec3(0.0, 0.0, -0.0), glm::vec3(1,1, 1)) , false);
    // readObj("../Data/bunny.obj", triangles, m, getTransformMatrix(glm::vec3(180, 0, 180), glm::vec3(200, -60, 150), glm::vec3(1500,1500, 1500)) , false);

    int nTriangles = triangles.size();
    std::cout << "Triangles: " << nTriangles << std::endl;

//    for(int id = 0;id < triangles.size();id++) {
//        std::cout << triangles[id].p1.x << " | " << triangles[id].p2.x << " | " << triangles[id].p3.x << std::endl;
//    }
    // ----------------------------------------------------------------------------------- //`

    std::vector<BVHNode> nodes;

    buildBVH(triangles, nodes, 0, triangles.size() - 1, 1);

    int nNodes = nodes.size();
    std::cout << "Nodes: " << nNodes << std::endl;

    // ---------------------------------------------------------------------------------- //

//    for(int id = 0;id < nodes.size();id++) {
//        std::cout << id << "  left:" << nodes[id].left << " right:" << nodes[id].right << "   |   "
//        << " AA: " << nodes[id].AA.x << ", " << nodes[id].AA.y << ", " << nodes[id].AA.z << " | "
//        << " BB: " << nodes[id].BB.x << ", " << nodes[id].BB.y << ", " << nodes[id].BB.z << std::endl;
//    }

    RayTracerShader.use();
    ObjTex.getTexture_Triangle(triangles, RayTracerShader, nTriangles);
    ObjTex.getTexture_BVH(nodes, RayTracerShader, nNodes);

	while (!glfwWindowShouldClose(window))
	{
		tRecord.updateTime();

		processInput(window);

		cam.LoopIncrease();

		{
			screenBuffer.setCurrentBuffer(cam.LoopNum);

			RayTracerShader.setInt("historyTexture", 0);

			ObjTex.setTex(RayTracerShader);

			RayTracerShader.use();

			RayTracerShader.setVec3("camera.camPos", cam.cameraPos);
			RayTracerShader.setVec3("camera.front", cam.cameraFront);
			RayTracerShader.setVec3("camera.right", cam.cameraRight);
			RayTracerShader.setVec3("camera.up", cam.cameraUp);
			RayTracerShader.setFloat("camera.halfH", cam.halfH);
			RayTracerShader.setFloat("camera.halfW", cam.halfW);
			RayTracerShader.setVec3("camera.leftbottom", cam.LeftBottomCorner);
			RayTracerShader.setInt("camera.LoopNum", cam.LoopNum);

			RayTracerShader.setFloat("randOrigin", 874264.0f * (GetCPURandom() + 1.0f));

			screen.DrawScreen();
		}

		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);

			ScreenShader.use();
			screenBuffer.setCurrentAsTexture(cam.LoopNum);

			ScreenShader.setInt("screenTexture", 0);

			screen.DrawScreen();
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	screenBuffer.Delete();
	screen.Delete();

	return 0;
}

void processInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cam.ProcessKeyboard(FORWARD, tRecord.deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cam.ProcessKeyboard(BACKWARD, tRecord.deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cam.ProcessKeyboard(LEFT, tRecord.deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cam.ProcessKeyboard(RIGHT, tRecord.deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	cam.updateScreenRatio(SCR_WIDTH, SCR_HEIGHT);
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
	cam.updateCameraFront(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	cam.updateFov(static_cast<float>(yoffset));
}






