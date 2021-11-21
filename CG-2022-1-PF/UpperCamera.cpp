#include "UpperCamera.h"

UpperCamera::UpperCamera() {}

UpperCamera::UpperCamera( GLfloat startMoveSpeed )
{
	xPos = 0.0f;
	zPos = 0.0f;

	position = glm::vec3(xPos, 300.0f, zPos);
	worldUp = glm::vec3(0.0f, 0.0f, 1.0f);
	front = glm::vec3(0.0f, -1.0f, 0.0f);

	
	moveSpeed = startMoveSpeed;
	//turnSpeed = startTurnSpeed;

}

void UpperCamera::keyControl(bool* keys, GLfloat deltaTime)
{
	GLfloat velocity = moveSpeed * deltaTime;

	if (keys[GLFW_KEY_W])
	{
		zPos += velocity;
	}

	if (keys[GLFW_KEY_S])
	{
		zPos -= velocity;
	}

	if (keys[GLFW_KEY_A])
	{
		xPos += velocity;
	}

	if (keys[GLFW_KEY_D])
	{
		xPos -= velocity;
	}

	update();
}

void UpperCamera::update()
{
	position = glm::vec3(xPos, 300.0f, zPos);
}

glm::mat4 UpperCamera::calculateViewMatrix()
{
	return glm::lookAt(position, position + front, worldUp);
}

glm::vec3 UpperCamera::getCameraPosition()
{
	return position;
}


glm::vec3 UpperCamera::getCameraDirection()
{
	return glm::normalize(front);
}


UpperCamera::~UpperCamera()
{
}
