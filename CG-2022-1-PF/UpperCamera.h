#pragma once

#include <glew.h>

#include <glm.hpp>
#include <gtc\matrix_transform.hpp>

#include <glfw3.h>

class UpperCamera
{
public:
	UpperCamera();
	UpperCamera(GLfloat startMoveSpeed);

	void keyControl(bool* keys, GLfloat deltaTime);

	glm::vec3 getCameraPosition();
	glm::vec3 getCameraDirection();
	glm::mat4 calculateViewMatrix();

	~UpperCamera();

private:
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 worldUp;

	GLfloat xPos;
	GLfloat zPos;

	GLfloat moveSpeed;
	//GLfloat turnSpeed;

	void update();

};

