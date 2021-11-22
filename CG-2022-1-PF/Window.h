#pragma once
#include<stdio.h>
#include<glew.h>
#include<glfw3.h>

class Window
{
public:
	Window();
	Window(GLint windowWidth, GLint windowHeight);
	int Initialise();
	GLfloat getBufferWidth() { return bufferWidth; }
	GLfloat getBufferHeight() { return bufferHeight; }
	GLfloat getXChange();
	GLfloat getYChange();
	bool getMainStart() { return mainStart; }
	void setMainStart(bool state) { mainStart = state; }
	bool getAni1Start() { return ani1Start; }
	bool getAni2Start() { return ani2Start; }
	bool getFreeCamera() { return freeCamera; }
	void setOffSpotLights(int n) { offSpotLights = n; }
	int getOffSpotLights() { return offSpotLights; }
	bool getShouldClose() {
		return  glfwWindowShouldClose(mainWindow);}
	bool* getsKeys() { return keys; }
	void swapBuffers() { return glfwSwapBuffers(mainWindow); }
	
	~Window();
private: 
	GLFWwindow *mainWindow;
	GLint width, height;
	bool keys[1024];
	GLint bufferWidth, bufferHeight;
	void createCallbacks();
	GLfloat lastX;
	GLfloat lastY;
	GLfloat xChange;
	GLfloat yChange;
	int offSpotLights;
	bool mainStart;
	bool ani1Start;
	bool ani2Start;
	bool mouseFirstMoved;
	bool freeCamera;
	static void ManejaTeclado(GLFWwindow* window, int key, int code, int action, int mode);
	static void ManejaMouse(GLFWwindow* window, double xPos, double yPos);

};

