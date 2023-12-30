#pragma once
#ifndef __Camera_h__
#define __Camera_h__

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

enum Camera_Movement {
    FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

class Camera {
public:
	Camera(int ScreenWidth, int ScreenHeight) {
		LoopNum = 0;
		Yaw = -90.0f;
		Pitch = 0.0f;
		fov = 22.5f;
		firstMouse = true;
		cameraPos = glm::vec3(278.0f, 273.0f, -800.0f);
		worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
		ScreenRatio = (float)ScreenWidth / (float)ScreenHeight;
		halfH = glm::tan(glm::radians(fov));
		halfW = halfH * ScreenRatio;
		cameraSpeed = 300.0f;
		updateCameraVectors();
	}

	void updateCameraVectors() {
		cameraFront.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		cameraFront.y = sin(glm::radians(Pitch));
		cameraFront.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		cameraFront = glm::normalize(cameraFront);

		cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));

		cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));

		LeftBottomCorner = cameraFront - halfW * cameraRight - halfH * cameraUp;

		LoopNum = 0;
	}
	void updateFov(float offset) {
		fov -= (float)offset;
		if (fov < 1.0f)
			fov = 1.0f;
		if (fov > 45.0f)
			fov = 45.0f;

		halfH = glm::tan(glm::radians(fov));
		halfW = halfH * ScreenRatio;

		LeftBottomCorner = cameraFront - halfW * cameraRight - halfH * cameraUp;

		LoopNum = 0;
	}
	void updateCameraFront(float xpos, float ypos) {
		if (firstMouse) {
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos;
		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.03f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// ��֤pitchС��90��
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;

		updateCameraVectors();
	}
	void updateScreenRatio(int ScreenWidth, int ScreenHeight) {
		ScreenRatio = (float)ScreenWidth / (float)ScreenHeight;
		halfW = halfH * ScreenRatio;

		LeftBottomCorner = cameraFront - halfW * cameraRight - halfH * cameraUp;

		LoopNum = 0;
	}
	void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
		float velocity = cameraSpeed * deltaTime;
		if (direction == FORWARD)
			cameraPos += cameraFront * velocity;
		if (direction == BACKWARD)
			cameraPos -= cameraFront * velocity;
		if (direction == LEFT)
			cameraPos -= cameraRight * velocity;
		if (direction == RIGHT)
			cameraPos += cameraRight * velocity;

		LeftBottomCorner = cameraFront - halfW * cameraRight - halfH * cameraUp;

		LoopNum = 0;
	}
	void LoopIncrease() {
		LoopNum++;
	}

public:
	glm::vec3 cameraPos;
	// �������
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	glm::vec3 cameraRight;
	// ���緽��
	glm::vec3 worldUp;
	float fov;
	float Pitch;
	float Yaw;
	// ����ƶ��ٶ�
	float cameraSpeed;
	// ��꽻�����
	bool firstMouse;
	float lastX;
	float lastY;
	// ��Ļ�����
	float ScreenRatio;
	float halfH;
	float halfW;
	glm::vec3 LeftBottomCorner;
	// ��Ⱦ������
	int LoopNum;
};


#include "glad/glad.h"
#include "GLFW/glfw3.h"

class timeRecord {
public:
	timeRecord() {
		lastFrameTime = 0.0f;
	}

	void updateTime() {
		currentFrameTime = static_cast<float>(glfwGetTime());
		deltaTime = currentFrameTime - lastFrameTime;
		lastFrameTime = currentFrameTime;
	}

	float currentFrameTime;
	float lastFrameTime;
	float deltaTime;

};






#endif




