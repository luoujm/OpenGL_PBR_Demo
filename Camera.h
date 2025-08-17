#pragma once
#ifndef CAMERA_H
#define CAMERA_H
#include <glad/glad.h>
#include <glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


class Camera {
public:
	Camera() {
		up = glm::vec3(0.0f, 1.0f, 0.0);
		cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
		
		//------------------------------------------------------------
		yaw = YAW;
		pitch = PITCH;
		MovementSpeed = SPEED;
		MouseSensitivity = SENSITIVITY;
		Zoom = ZOOM;
	
		updateCameraVectors();
	}
	Camera(const glm::vec3 &Pos) {
		up = glm::vec3(0.0f,1.0f,0.0);
		cameraPos = Pos;
		
		//-------------------------------------------------------------
		yaw = YAW;
		pitch = PITCH;
		MovementSpeed = SPEED;
		MouseSensitivity = SENSITIVITY;
		Zoom = ZOOM;

		updateCameraVectors();
	}
	Camera(const glm::vec3& Pos,const glm::vec3& UP){
		up = UP;
		cameraPos = Pos;
		
		//-------------------------------------------------------------
		yaw = YAW;
		pitch = PITCH;
		MovementSpeed = SPEED;
		MouseSensitivity = SENSITIVITY;
		Zoom = ZOOM;

		updateCameraVectors();
	}
	glm::mat4 GetViewMatrix() {
		return glm::lookAt(cameraPos,cameraPos+cameraFront,up);//摄像机位置，摄像机朝向点，世界坐标的上向量
	}

	

	void CameraMove(Camera_Movement direction) {

		float cameraSpeed = MovementSpeed * deltaTime;// adjust accordingly,把移动量转化为时间-固定了在任意系统上运行的移动速度
		if (direction==FORWARD)
			cameraPos += cameraSpeed * cameraFront;
		if (direction==BACKWARD)
			cameraPos -= cameraSpeed * cameraFront;
		if (direction == LEFT)
			cameraPos -= cameraRight * cameraSpeed;
		if (direction == RIGHT)
			cameraPos += cameraRight * cameraSpeed;
		if (direction == UP)
			cameraPos -= cameraUp * cameraSpeed;
		if (direction == DOWN)
			cameraPos += cameraUp * cameraSpeed;
		

	}

	void Refresh(float time) {

		deltaTime = time - lastFrame;
		lastFrame = time;
		
	}
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
		
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		yaw += xoffset;
		pitch += yoffset;
		if (constrainPitch) {
			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;
		}
		updateCameraVectors();
	}
	void ProcessMouseScroll(float yoffset)
	{
		Zoom -= (float)yoffset;
		if (Zoom < 1.0f)
			Zoom = 1.0f;
		if (Zoom > 45.0f)
			Zoom = 45.0f;
	}

	glm::vec3 cameraPos;
	
	glm::vec3 cameraFront;
	glm::vec3 up;
	glm::vec3 cameraDirection;
	glm::vec3 cameraRight;
	glm::vec3 cameraUp;
	float Zoom;
	float yaw = -90.0f, pitch = 0.0f;
private:
	void updateCameraVectors() {

		glm::vec3 front;
		front.y = sin(glm::radians(pitch));
		front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
		cameraFront =front;
		cameraDirection = cameraFront;
		cameraRight = glm::normalize(glm::cross(cameraDirection,up));
		cameraUp = glm::cross(cameraDirection, cameraRight);
	
	}
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	
	float MovementSpeed;
	float MouseSensitivity;
	
};


#endif